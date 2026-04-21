# GPU 视角

GPU 的基本结构和 TPU 很接近：都有专门做矩阵乘法的单元，都依赖 HBM 供给数据，也都需要把通信和计算一起纳入 roofline。差异主要在于，GPU 的计算单元更多、层级更碎，网络也更依赖节点和交换机拓扑。

## 芯片结构

现代 ML GPU 可以粗略看成很多个 SM 组成的阵列。每个 SM 内部都有面向矩阵乘法的 Tensor Core、负责逐元素和归约的 CUDA cores，以及靠近计算单元的片上缓存或寄存器文件。和 TPU 相比，GPU 更像是很多个较小的计算岛，而不是少数几个更大的矩阵核心。

这意味着两件事：

- GPU 单块芯片更灵活，能并发处理更多独立任务。
- 但要把一个大 matmul 喂满所有 SM，往往更依赖内核调度和缓存组织。

## 节点与互联

节点内通常通过 NVLink 和 NVSwitch 形成高带宽互联。以 H100 为例，单 GPU 的对外 egress 约为 `450 GB/s`，而 B200 则可到 `900 GB/s`。节点外则进入 InfiniBand 或更大的 scale-out fabric，常见的节点级 egress 约为 `400 GB/s`。

这两个数字决定了很多并行策略的边界：

- 节点内通信通常比跨节点便宜。
- 节点外 collective 往往更快进入通信受限。
- GB200 NVL72 通过更大的 NVLink domain 抬高了节点级 egress，使跨节点 roofline 发生变化。

在拓扑上，GPU 集群通常是 fat tree，而不是 TPU 那种更规则的最近邻网络。于是，collective 的代价会更强地依赖节点大小、交换机层数和是否有足够的双向带宽。

## 集体通信

GPU 上同样常用 `AllGather`、`ReduceScatter`、`AllReduce` 和 `AllToAll`。在节点内，这些操作通常可以按 ring 近似分析；在节点外，还要额外看交换机层和链路层是否能提供足够的 bisection bandwidth。

对单节点 `B` 字节的 `AllGather` 或 `ReduceScatter`，吞吐下界大致是：

$$
T_{comm} \approx \frac{B}{W_{GPU\ egress}}
$$

`AllReduce` 若没有 in-network reduction，通常可近似看成两倍成本。`AllToAll` 在节点内相对更便宜，因为 GPU 节点本身就是 all-to-all 连接，但在节点外会迅速受拓扑和延迟限制。

这里和 TPU 的差别在于，GPU 的带宽数字虽然也很大，但多层交换网络会让“理论总带宽”与“实际可用带宽”之间出现更明显的落差。

## LLM roofline

GPU 上最重要的两个门槛是 data parallelism 和 tensor parallelism。

### Data Parallelism

纯 DP 或 ZeRO 类训练需要在反向对梯度或权重做 collective。若以 H100 的 bf16 峰值和 450 GB/s 节点内带宽估计，单 GPU 需要大约：

$$
\frac{990 \times 10^{12}}{450 \times 10^9} \approx 2200
$$

tokens 才能接近 compute-bound；跨节点时约为：

$$
\frac{990 \times 10^{12}}{400 \times 10^9} \approx 2475
$$

tokens。MoE 会再乘上 `E / k`，因为激活参数少于总参数。

结论是：GPU 上 DP 也需要足够大的 token batch，否则梯度通信很容易先成为瓶颈。

### Tensor Parallelism

TP 需要在激活上做 `AllGather` 和 `ReduceScatter`。若 feed-forward 维度为 `F`，则大致要求 TP 轴大小不超过 `F / 2200` 到 `F / 2475` 的量级，具体取决于通信发生在节点内还是节点外。对 LLaMA-3 这类 `F ≈ 28000` 的模型，这意味着大约 8-way 到 11-way 的区间已经接近上限。

因此，TP 往往适合单节点或少量节点。超过这个范围后，通信开销上升得很快。

### Expert Parallelism

MoE 让 EP 变得有吸引力，因为它把总参数放大到 `E`，但每个 token 只激活 `k` 个专家。实现上通常需要两次 `AllToAll` 把 token 发给对应专家，再把结果送回。

EP 是否划算，主要取决于 `F` 是否足够大，以及 `E/k` 带来的稀疏性是否足以抵消通信。小 `F` 下，EP 往往只能跨 1-2 个节点；大 `F` 下，它可以跨更多节点，并且通常比继续扩大 TP 更稳妥。

### Pipeline Parallelism

PP 的通信量很小，因为它只传激活微批次。它的问题不在带宽，而在调度：

- 会引入 pipeline bubble。
- 会让 ZeRO-3 这类需要频繁 `AllGather` 权重的方案变得更难用。
- 需要更复杂的 forward/backward 交错。

因此，PP 通常是“通信便宜、实现贵”。它适合作为跨节点的层切分手段，但不适合把复杂度当作免费的优化。

## 组合策略

GPU 上真正可行的方案通常不是单一并行，而是组合：

- 小而 dense 的模型，可以靠 DP 或 FSDP 解决。
- 更大的 dense 模型，常见组合是少量 TP + 多层 PP + 外层 DP。
- MoE 模型通常还会加入 EP，因为它比继续扩 TP 更符合稀疏结构。

典型训练配置可以说明这一点：

- DeepSeek V3 用了大规模 EP、PP 和少量 DP，把模型并行拉到了很高的层数。
- LLaMA-3 则用较强的 TP + PP + DP，在 dense 模型里把批大小和通信压力维持在可接受范围。

这些方案的共同点是：先让模型并行跨越足够多的层，再把 DP 或 ZeRO 的通信代价摊薄。

## 结论

可以把结论压缩成几条：

- GPU 的算力足够强，但单节点和跨节点的带宽边界很明显。
- DP、TP、EP、PP 的选择，本质上是在不同 collective 成本之间做交换。
- 节点内通信通常便宜于节点外，但 GPU 网络的层级更多，延迟更容易影响实际吞吐。
- 对 LLM 而言，能否算得快，取决于 batch、模型切分和网络拓扑是否一起匹配。

若把 TPU 和 GPU 放在同一 roofline 框架下比较，GPU 侧只是把同样的分析方法放到更复杂的网络层级上。公式并没有变，变化的是哪一层最先成为瓶颈。

## 补充算例

- 8xH100 节点上，对 `bf16[1024, 16384]` 做 `AllGather`，理论下界大约在 `65` 到 `75` 微秒量级。这个结果说明，只有消息足够大时，节点级带宽才会接近峰值；小消息更容易受延迟和实现细节影响。

- 两个节点之间做精确的 `AllGather` 时，节点内带宽仍可能先成为瓶颈。对 8 卡 H100 节点而言，节点级上界约为 `B / 514GB/s`，而跨节点上界约为 `B / 800GB/s`，因此更大的 fabric 并不自动意味着更高吞吐。

- 对节点内的 `AllToAll`，稠密情形的成本大致是 `B / (8 * W)`；如果只有一半条目非零，成本会近似减半。对于 MoE 这类稀疏路由，通信量往往更接近非零比例，而不是张量名义大小。

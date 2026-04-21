# 训练并行

## 并行目标

训练并行关注的不是“能不能把模型跑起来”，而是“增加芯片数后，吞吐能否近似线性增长”。

在 TPU 上，瓶颈通常来自两件事：

- 单卡上的计算是否足够多，能否掩盖通信。
- 跨卡通信是否比本地 matmul 更慢。

记号上，`B` 表示总 token 数，`D` 和 `F` 分别是 `d_model` 与 `d_ff`，`C` 是单芯片 FLOPs/s，`W_ici` 是 ICI 带宽。对 TPU v5p，`C / W_ici` 的量级约为 `2550`，这可以直接当作网络的算术强度阈值。

## 数据并行

纯数据并行只切 batch，参数和优化器状态全部复制。

- 前向不通信。
- 反向只在梯度上做 `AllReduce`。
- 通信不在关键路径上，比较容易实现重叠。

它的极限由每卡 batch 决定。对 bf16 权重和 TPU v5p，若

$$
\frac{B}{X} < \frac{C}{W_{ici}}
$$

就会变成通信受限。也就是说，单卡 token 数低于约 2550 时，纯数据并行很容易被 ICI 拖住。

但纯 DP 复制参数和优化器状态，内存很快先成为问题。按 bf16 参数加 fp32 Adam 状态估算，约等于每参数 10 字节，因此它只适合参数规模还不大的模型。

## FSDP

FSDP 的变化是把参数和优化器状态也分片。

- 前向前对权重做 `AllGather`。
- 反向后对梯度做 `ReduceScatter`。
- 通信总量和把梯度做 `AllReduce` 本质等价。

它和纯 DP 的关键区别在内存，不在通信形状。由于 `AllReduce = AllGather + ReduceScatter`，所以 FSDP 仍然服从同一个通信阈值：

$$
\frac{B}{X} \gtrsim \frac{C}{W_{ici}}
$$

这意味着 FSDP 适合大模型存储，但不会自动解决小 batch 的通信瓶颈。

## Tensor Parallelism

Tensor parallelism 把激活切在 `D` 维，把权重切在 `F` 维。

- 前向先 `AllGather` 激活。
- 经过 `W_in` 后再做局部 matmul。
- 经过 `W_out` 后 `ReduceScatter` 输出。

它的通信量和 batch 无关，主要由 `F` 决定，因此适合中等 batch、较大 hidden size 的情形。对 TPU v5p，经验阈值可以写成：

$$
Y > M_Y \cdot \frac{F}{2550}
$$

其中 `M_Y` 是可用的 ICI 轴数。对常见模型，这对应大约 8 到 16-way 的 tensor parallelism。

## 混合 FSDP 与 TP

FSDP 移动的是权重，TP 移动的是激活。两者结合后，通信不再随 batch 线性变化，而更接近平方根规律。

在总芯片数固定时，最优划分是让两条轴上的通信时间尽量相等。工程上可以把它理解为：FSDP 负责把大权重切薄，TP 负责把大激活切薄。

在 `F ≈ 32768`、三维 mesh、`C / W_ici ≈ 2550` 的设置下，混合方案可以把可保持 compute-bound 的每卡 batch 压到约 100 token 量级，而纯 FSDP / 纯 DP 仍通常要到 850 左右。这是训练并行里最关键的中间区间。

## 补充算例

- 以 LLaMA-2 13B 为例，`D=5120`、`F=13824`、`L=40`、`V=32000` 时，FFW 约 `8.5B` 参数，attention 约 `4.2B`，词表约 `0.33B`，总量与 13B 级别一致。
- 若 batch 为 `16M` token，bf16 参数加 fp32 Adam 状态约占 `130GB`，而激活 checkpoint 约为 `42TB`；此时内存主要被激活而不是参数占据。
- 在 `32k` 序列长度、`3M` token batch、`16x16x16` TPU v5p 切片上，纯 DP 先被参数内存卡住，纯 FSDP 则会落入通信受限，混合 FSDP + TP 才能把每卡 batch 压回可计算区间。

## 流水并行

流水并行把层切到不同设备上，靠 microbatch 填平 bubble。

- 优点是跨 stage 通信较轻。
- 缺点是 bubble 管理复杂，且容易出现某一端长期空转。

它更像 GPU 集群里的主力方案；在 TPU 上，由于 pod 内互联更强，通常先考虑 FSDP 和 TP 的组合，再考虑流水。

## 跨 Pod

一旦跨越 pod，就进入 DCN 层。DCN 带宽远低于 ICI，因此不能把它当成和片内通信同一数量级的成本。

经验上，若要在 DCN 上保持纯数据并行不被拖慢，每个 pod 需要至少约 `71k` token 的批量。更现实的做法是：

- pod 内用 FSDP / TP 把计算尽量做密。
- pod 间只保留更粗粒度的数据并行。

## 结论

- batch 越小、并行越多，越容易通信受限。
- 纯 DP 适合大 batch 但内存利用率差。
- FSDP 解决存储，不自动解决通信。
- TP 适合中等 batch，常见规模大约是 8 到 16-way。
- 混合 FSDP + TP 是训练大模型时最实用的中间方案。
- 跨 pod 以后，通信层级会从 ICI 退化到 DCN，阈值会显著抬高。

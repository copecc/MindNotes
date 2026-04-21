# Roofline 与矩阵乘法

Roofline 模型提供了一种统一的性能分析框架，用于判断算子的主瓶颈更接近计算吞吐还是数据搬运。对 LLM 而言，这个框架最常见的用途是分析矩阵乘法、量化后的线性层以及分布式并行时的通信开销。

## Roofline 视角

分析一个算子的时间时，通常先拆成两部分：

- 计算时间 `T_math`
- 通信时间 `T_comms`

这里的通信既可以指片内 HBM 与计算单元之间的数据搬运，也可以指跨芯片网络通信。

如果计算与通信可以充分重叠，总时间的一阶下界接近：

$$
\boxed{T \approx \max(T_{math}, T_{comms})}
$$

如果二者完全不能重叠，总时间上界接近：

$$
T_{math} + T_{comms}
$$

因此工程上常先用 `max(T_math, T_comms)` 估计瓶颈位置，再根据 profiling 判断重叠区域与额外开销。

由此可以得到两个基本结论：

- 当 `T_math > T_comms` 时，算子更接近 **compute-bound**。
- 当 `T_comms > T_math` 时，算子更接近 **communication-bound**。

Roofline 的核心不只是区分这两类状态，而是解释瓶颈为什么出现，以及改变数据形状、精度或并行方式后，瓶颈是否会转移。

## 算术强度

判断瓶颈最常用的量是 **算术强度**，也即 arithmetic intensity 或 operational intensity。它定义为：

$$
I = \frac{\text{total FLOPs}}{\text{total bytes moved}}
$$

它表示单位字节搬运对应了多少计算。算术强度越高，说明同一批数据被复用得越充分，算子越有可能受计算吞吐限制；算术强度越低，说明更多时间花在搬运数据上。

硬件也有一个对应的临界强度：

$$
I_{crit} = \frac{\text{peak FLOPs/s}}{\text{bandwidth}}
$$

当 `I < I_crit` 时，算子更容易受带宽限制；当 `I > I_crit` 时，算子更容易进入 compute-bound 区域。

以 TPU v5e MXU 为例，其峰值算力约为 `1.97e14 FLOPs/s`，HBM 带宽约为 `8.2e11 bytes/s`，因此临界强度约为：

$$
I_{crit} \approx 240 \text{ FLOPs/byte}
$$

这个值可以理解为一个分界点：低于该值时，提高计算单元峰值通常帮助有限；高于该值时，继续提高带宽的收益反而更小。

### 低算术强度示例

以 bfloat16 dot product 为例，`x, y \in \mathbb{R}^N`：

- 读取两个长度为 `N` 的向量，需要约 `4N` 字节。
- 执行 `N` 次乘法和 `N - 1` 次加法，总 FLOPs 约为 `2N`。

因此当 `N` 足够大时，其算术强度近似为：

$$
I \approx \frac{2N}{4N} = 0.5
$$

这个值远低于大多数硬件的临界强度，因此 dot product 往往天然更接近通信受限。这个例子说明，问题不在于 FLOPs 总量绝对不大，而在于每搬运 1 字节数据时做的计算太少。

## Roofline 图的含义

Roofline 图通常以算术强度为横轴，以可达到的吞吐为纵轴。其结构可以概括为两段：

- 左侧斜线区域：带宽受限，吞吐近似与算术强度线性相关。
- 右侧平台区域：计算受限，吞吐被峰值 FLOPs/s 封顶。

这张图表达的不是某个算子“理论上有多少 FLOPs”，而是它在给定硬件上最多能以多高的有效吞吐运行。提升性能的常见方式也对应这两部分：

- 提高数据复用，增加算术强度。
- 提高可用带宽。
- 若已进入平台区，则只能依赖更高的峰值算力或更少的总计算量。

## 矩阵乘法

对矩阵乘法：

$$
X[B, D] @ Y[D, F] \rightarrow Z[B, F]
$$

如果输入和权重都采用 bfloat16，则：

- 读取激活 `X` 需要约 `2BD` 字节。
- 读取权重 `Y` 需要约 `2DF` 字节。
- 写回输出 `Z` 需要约 `2BF` 字节。
- 总 FLOPs 近似为 `2BDF`。

因此算术强度可写成：

$$
I = \frac{2BDF}{2BD + 2DF + 2BF}
$$

如果 `B` 相对 `D`、`F` 较小，而 `D`、`F` 本身足够大，则分母通常主要由 `2DF` 主导，此时有近似：

$$
I \approx \frac{2BDF}{2DF} = B
$$

这说明在常见 Transformer 场景下，矩阵乘法的算术强度与局部 batch 规模近似同阶增长。也正因如此，matmul 往往随着局部 token 数增大而从带宽受限转向计算受限。

### 为什么 matmul 常常更容易 compute-bound

矩阵乘法的特点不是总计算量大，而是权重和激活可以在块内反复复用。每搬一次数据，往往能支撑大量乘加操作。这种高复用结构使它和 dot product、逐元素算子表现出很不相同的 roofline 特征。

如果仍以 TPU v5e 的 `240 FLOPs/byte` 作为近似阈值，并使用 `I \approx B` 的近似，则可以得到一个直接结论：

- 对 bfloat16 matmul，**per-replica token batch 大于约 240 时，更容易进入 compute-bound 区域**。

这个阈值不是普适常数，但它给出了一个很有用的一阶工程判断。

## per-replica token batch

在 Transformer 中，许多线性层虽然输入张量看起来是：

$$
[B, S, D]
$$

但在进入矩阵乘法前，常被重排为：

$$
[(B \times S), D]
$$

这意味着底层硬件真正看到的“行数”不是序列条数，而是本卡上实际参与运算的 token 总数。于是对 matmul 的 roofline 判断，更关键的量不是 sequence batch size，而是 **per-replica token batch size**。

这一区分带来两个直接结论：

- 短序列任务往往需要更大的序列批大小，才能凑出足够多的 token。
- 长序列任务即使 sequence batch 很小，也可能已经有足够高的算术强度。

例如，若全局共有 512 条序列，每条长度 4096，总共使用 128 张卡，则：

$$
\text{global tokens} = 512 \times 4096 = 2{,}097{,}152
$$

$$
\text{local tokens} = 2{,}097{,}152 / 128 = 16{,}384
$$

这个单卡 token 数远高于 240，因此就片内 HBM roofline 而言，这类 matmul 通常更容易接近 compute-bound。

## 量化对 roofline 的影响

量化会同时改变字节搬运量和可用算力，因此对 roofline 的影响不能只看“参数是否变小”。

### int8 权重与 int8 激活

若把 bfloat16 matmul 改为 int8 matmul，则：

- 每个参数从 2 字节下降到 1 字节。
- 总 FLOPs 仍与 `2BDF` 同阶。
- 硬件可用的 int8 峰值 OPs/s 往往高于 bfloat16。

这时临界强度本身也会上升。若取 HBM 带宽约为 `8.1e11 bytes/s`、int8 峰值吞吐约为 `3.94e14 OPs/s`，则：

$$
I_{crit}^{int8} \approx 486
$$

如果仍在 `B \ll D, F` 的近似下分析，则 matmul 的算术强度近似变成 `2B`，于是临界条件约为：

$$
2B > 486 \Rightarrow B > 243
$$

这个结果说明，虽然权重与激活字节数减半，但临界 batch 与 bfloat16 情况相比并没有发生数量级变化。原因在于，峰值算力与带宽刻画的临界强度也同步变化，二者产生了部分抵消。

### int8 权重与 bf16 激活

另一类更常见的情形是：

$$
bf16[B, D] * int8[D, F] \rightarrow bf16[B, F]
$$

这里的特点是：

- 权重搬运字节数下降。
- 激活与输出仍保持较高精度。
- 计算吞吐仍按 bfloat16 路径估计。

在 `B \ll D, F` 的近似下，读取字节数从 `2BD + 2DF + 2BF` 变为约 `2BD + DF + 2BF`。由于权重项从 `2DF` 降为 `DF`，算术强度会明显上升，因此更容易在较小 batch 下进入 compute-bound。

这也是低精度存权重、高精度做计算常常能带来效率收益的原因之一。它不一定改变总 FLOPs，但能降低搬运成本，从而提高每字节对应的计算量。

### 每个 batch 元素使用独立权重矩阵

若结构变成：

$$
int8[B, D] * int8[B, D, F] -> int8[B, F]
$$

则问题会明显不同。此时权重不再在 batch 维度共享，而是随着 `B` 一起增长。结果是：

- 总 FLOPs 仍与 `2BDF` 同阶。
- 总通信量也会随着 `BDF` 同阶增长。
- 算术强度不再主要由 batch 增长拉高，而更接近常数。

这种结构通常很难通过简单增大 batch 摆脱通信瓶颈，因为新增的计算几乎总是伴随同比例的新增权重搬运。

## 分块与片上存储

矩阵乘法分析先默认每个输入块只被读取一次，但实际硬件执行大矩阵乘法时，通常需要把计算拆成更小的 tile，并借助片上存储反复复用数据。对 TPU 而言，资料中常会提到 `VMEM`、`SMEM`、`TMEM` 等片上存储层级。

这里最关键的结论不是区分缩写本身，而是理解其 roofline 含义：

- 片上存储复用越充分，对 HBM 的重复访问越少。
- tile 设计越差，越容易把本可 compute-bound 的算子重新拉回 memory-bound。

因此，理论上由 `2BDF / (2BD + 2DF + 2BF)` 得到的算术强度，往往是一个偏理想化的上界估计。真实实现还会受到 tile 大小、片上缓存容量和复用路径的影响。

## 跨芯片通信 roofline

Roofline 不只适用于单芯片内部。对分布式训练和推理，另一个同样重要的约束面是 **跨芯片网络 roofline**。

考虑将矩阵乘法：

$$
X[B, D] @ Y[D, F]
$$

沿 `D` 维切分到两个设备。每个设备先计算本地部分和，再把结果通过网络交换并累加。此时：

- 每个设备只执行部分 FLOPs，因此 `T_math` 降低。
- 但每个设备都必须发送部分和，因此新增了跨卡 `T_comms`。

在这个例子里，是否 compute-bound 取决于：

- 局部计算量与跨卡通信量的比值
- interconnect 的有效带宽

在这种沿 `D` 维切分的场景下，临界条件可能主要依赖 `D`，而不是 `B`。更具体地说，若每个设备的 FLOPs 与需要交换的部分和之比近似为 `D / 2`，则是否进入 compute-bound 区域要看：

$$
\frac{D}{2} > \frac{\text{peak FLOPs/s}}{\text{network bandwidth}}
$$

这说明并行化之后，roofline 的主瓶颈可能已经不是单卡 HBM，而是跨卡网络。一个在单卡上已经 compute-bound 的 matmul，扩展到多卡后仍然可能重新变成 communication-bound。

因此分布式场景至少需要同时考虑两层 roofline：

- 片内 HBM roofline
- 跨芯片网络 roofline

二者约束的是不同的数据路径，不能互相替代。

## GPU 的对应判断

同样的分析也适用于 GPU。以 H100 SXM 为例，若去掉结构化稀疏性宣传口径中的翻倍因子，bfloat16 峰值 FLOPs/s 约可按 `1e15` 估计，HBM 带宽约为 `3.35e12 bytes/s`，则其临界强度大约为：

$$
\frac{1e15}{3.35e12} \approx 298
$$

这意味着对 bfloat16 matmul，GPU 上的临界 token batch 往往略高于 TPU，但仍在同一数量级。也就是说，很多关于“单卡 token 数是否足够大”的判断，在 GPU 与 TPU 上都成立，只是阈值略有差异。

## 实践判断

使用 roofline 分析 LLM 算子时，可以按以下顺序判断：

- 先估算总 FLOPs。
- 再估算总字节搬运量。
- 计算算术强度，并与目标硬件的临界强度比较。
- 判断瓶颈位于片内 HBM、片上存储复用，还是跨芯片网络。
- 最后再看计算与通信能否充分重叠。

对 LLM 中最常见的 matmul，有几个稳定结论：

- 单卡 token 数越大，越容易进入 compute-bound。
- 量化是否有效，不只取决于精度是否降低，还取决于字节数下降与峰值算力变化之间的关系。
- 权重若无法在 batch 维度共享，算术强度通常更难提高。
- 单卡 roofline 与多卡 roofline 是两个不同问题，前者受 HBM 约束，后者常受 interconnect 约束。

Roofline 的价值不在于记住某个固定阈值，而在于建立一套统一的分析顺序：算多少、搬多少、在哪里搬、能否重叠，以及瓶颈是否随着数据布局、精度和并行方式改变。

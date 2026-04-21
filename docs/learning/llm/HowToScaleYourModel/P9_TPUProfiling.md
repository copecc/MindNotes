# TPU Profiling

TPU profiling 的目标不是“看一张图”，而是把高层 JAX 代码、编译后的 HLO、实际执行的 trace 和内存占用对应起来。只有这样，才能判断慢在算子、布局、通信，还是调度。

## 编译链路

JAX 代码经过 `jax.jit` 后，会先降低到 StableHLO，再经 XLA 优化为 HLO，最后继续降到更接近 TPU 执行的低层表示。这个链路里，最重要的是三件事：

- 逻辑计算有没有被正确识别成 matmul、点运算或 collective。
- 张量 layout 和 tiling 是否引入了额外 copy。
- sharding 是否把本来该本地完成的工作变成了通信。

所以，性能分析不是只看 Python 源码，而是要看编译后真正执行了什么。

## Trace Viewer

Trace Viewer 是最直接的入口。它提供的是时间轴：

- 每个 core 什么时候在算。
- 每个 XLA op 运行了多久。
- 哪些阶段是 matmul，哪些阶段是 collective，哪些阶段是空泡。

对 Transformer 来说，Trace Viewer 通常能直接显示重复的 layer 结构，因此可以很快判断：

- 哪一段是 attention。
- 哪一段是 MLP。
- 中间是否插入了不必要的通信。

如果一个 layer 的某段时间明显拉长，通常先看它对应的 HLO，再看是否发生了意外的 sharding 或 layout 变换。

## XLA op 语义

HLO 的字符串并不难读，核心是识别四类信息：

- shape，例如 `bf16[32,32,4096]`。
- layout 和 tiling，例如 `{2,1,0:T(8,128)(2,1)}`。
- memory space，例如 `S(1)` 代表 VMEM，`S(0)` 往往代表 HBM。
- op 类型，例如 `dot`、`fusion`、`all-reduce`、`reduce-scatter`。

这类信息足以回答很多关键问题：

- 这个 op 真的是 matmul，还是只是融合了点操作。
- 这个张量到底在哪个存储层。
- 这个数组有没有因为 tiling 导致 padding。

如果 layout 不合适，XLA 可能插入 retile 或 re-layout copy。很多“莫名其妙慢”的问题，其实都出在这里。

## Graph Viewer

Graph Viewer 的价值在于把 HLO 变成更容易读的 DAG。它特别适合检查两类问题：

- 一个 op 的输入到底来自哪里。
- 某段 collective 是否真的是必要的数据依赖。

当 trace 看起来像“一个大块 fusion 很慢”时，Graph Viewer 往往能更清楚地暴露：慢的不是 fusion 本身，而是前后某个 copy、collective 或 layout 迁移。

## 真实 profile

真实 Transformer profile 的常见模式是：

- MLP 由大 matmul 主导。
- attention 夹杂更多 memory traffic 和 smaller collectives。
- 某些层之间会出现通信尾巴，尤其在分片不理想时。

在示例 profile 里，某个 MLP up-projection 的本地 shard 是 `bf16[8,1024,8192] @ bf16[8192,16384]`，而全局实际形状更大；通过最终的 `all-reduce` 可以反推出真实的 model parallel 切分。这个过程的关键不是记住某个具体数字，而是学会从 shard 形状反推全局形状。

对这类 profile，常见检查顺序是：

1. 先看单个 matmul 的理论时间是否接近 roofline。
2. 再看 collective 是否超过了它应有的量级。
3. 最后确认 sharding 是否符合预期。

如果某个 collective 比估计值大很多，通常意味着：

- 设备拓扑不理想。
- tensor 太小，进入了 latency-bound 区域。
- sharding 方式让通信路径变长了。

## 内存视图

Memory Profile 用来找 OOM 和峰值内存。它最有价值的地方不是显示“总共用了多少”，而是显示：

- 参数占了多少。
- KV cache 占了多少。
- 峰值是否出现在某个 collective 或某次临时缓冲区创建时。

对推理和训练都一样，很多内存问题不是静态总量，而是峰值时刻的重叠导致的。只看最终驻留量，往往会漏掉真正的 OOM 原因。

## 工程用法

TPU profiling 的实用流程通常是：

- 先用 Trace Viewer 找慢点。
- 再用 HLO 和 Graph Viewer 解释慢点。
- 再回到代码里修 sharding、layout 或 fusion。
- 最后用 Memory Profile 验证峰值内存有没有下降。

在 JAX 里，`with_sharding_constraint` 往往是修正 GSPMD 误判的关键工具。很多时候不是硬件不行，而是编译器替你选了不合适的分片。

## 工程结论

Profiling 的核心不是“找到一个慢 op”，而是建立三层对应关系：

- 代码在表达什么。
- HLO 实际编译成了什么。
- TPU 真正执行了什么。

只有这三层都对齐，才能判断性能问题到底来自算力、带宽、通信，还是 layout。

## 补充算例

- 在一个 FFW profile 里，如果局部 shard 显示 `bf16[8,1024,8192] × bf16[8192,16384] -> bf16[8,1024,16384]`，并且末尾接的是 `ReduceScatter`，则全局形状可还原为 `bf16[32,1024,8192] × bf16[8192,32768] -> bf16[32,1024,32768]`。
- 这类层的计算时间若约为 `95.6ms`，而对应的 `ReduceScatter` 只有约 `1.1ms`，说明主瓶颈已经接近 roofline，后续优化应优先检查分片策略和布局，而不是继续盲目拆算子。
- 读 profile 时，先从局部 shard 反推全局 batch 和 hidden 维度，再看 collective 的代价是否显著偏离估计值，通常比只盯着单个 matmul 更有效。

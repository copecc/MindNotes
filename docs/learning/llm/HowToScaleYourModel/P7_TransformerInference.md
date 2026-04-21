# Transformer 推理

Transformer 推理和训练不是同一类问题。训练关心整体吞吐，推理同时关心 `TTFT`、单 token 延迟、请求并发和上下文长度。对系统设计而言，关键差别在于：推理会反复复用历史 token，因此必须显式管理 `KV cache`。

## 推理结构

最朴素的采样方式会在每一步重新计算整个前缀。若生成长度为 `n`，总代价接近二次复杂度。这种实现可以工作，但不会用于生产。

加入 `KV cache` 后，推理被拆成两个阶段：

- `Prefill`：一次性处理 prompt，生成并保存各层的 key/value。
- `Generation`：每次只输入新 token，利用历史 `KV cache` 计算下一个 token。

这样之后，历史 token 不再被重复前向，生成的主代价从“重算前缀”转为“读取和追加缓存”。复杂度仍然线性增长，但瓶颈位置已经改变。

## 吞吐与延迟

推理优化不再只看吞吐，还要看延迟指标：

- `TTFT`：首 token 到达时间，主要受 `Prefill` 影响。
- `Per-token latency`：后续 token 的平均生成延迟，主要受 `Generation` 影响。

`Prefill` 更接近训练：prompt 往往足够长，线性层通常更容易进入 compute-bound。`Generation` 更接近在线服务：每一步只处理少量新 token，batch 更小，硬件更难被填满。

对矩阵乘法，若 `B \ll D, F`，则算术强度近似满足 `I \approx B`。在 TPU v5e 上，bf16 的临界强度约为 `240 FLOPs/byte`，因此：

- `Prefill` 中，prompt token 足够多时，线性层通常能接近 compute-bound。
- `Generation` 中，只有当 per-replica token batch 足够大时，才更容易进入 compute-bound。

这也是为什么推理阶段对 batch 的约束比训练更强：batch 太小会直接暴露 HBM 带宽和通信开销。

## 记忆开销

推理时的主要内存由三部分组成：

- 模型参数。
- `KV cache`。
- 峰值工作区间，通常远小于前两者，尤其在使用 FlashAttention 时。

和训练不同，推理不需要 optimizer state 和反向激活 checkpoint。于是内存主导项通常变成 `KV cache`。其大小随层数、head 数、head 维度、序列长度和 batch 线性增长，形式上可写成：

$$
\text{KV size} \propto 2 \cdot L \cdot K \cdot H \cdot \text{seq\_len} \cdot \text{batch}
$$

这带来几个直接后果：

- 长上下文会迅速抬高显存占用。
- 生成 batch 不能只按吞吐扩张，还要受 `KV cache` 容量约束。
- 降低 `KV cache` 大小，通常比单纯压缩参数更能改善生成吞吐。

常见的减小 `KV cache` 方法包括：

- `GQA/GMQA`：减少 KV head 数，让多个 Q head 共享同一组 KV。
- 局部注意力：限制单层可见上下文长度。
- 跨层共享 KV：减少重复存储，但可能增加 HBM 读取次数。
- 量化：降低参数和 KV 的字节搬运。
- ragged reads / paged attention：避免为 padding 付费。

## 多卡推理

推理分布式化时，`Prefill` 和 `Generation` 的最优策略通常不同。

`Prefill` 的行为接近训练，因此可以采用训练阶段常见的并行方式：模型并行、序列并行、流水并行，甚至更复杂的切分。只要通信能被足够的计算掩盖，这些方法都可用。

`Generation` 的约束更强。它通常更偏 memory-bound，也更怕通信开销，因此：

- 不适合把权重频繁从慢网络搬来搬去。
- 更适合做模型并行，而不是 FSDP 式的参数搬运。
- 当 batch 较小或上下文较长时，序列切分 `KV cache` 可能成为必要选择。

核心原则是：推理时应尽量移动激活和少量中间结果，而不是移动权重本身。

## 服务组织

最简单的在线服务是交替执行一批 `Prefill` 和一批 `Generation`。这种方案实现容易，但有四个明显问题：

- `TTFT` 很差，因为所有 prompt 必须先完成。
- 长请求会阻塞短请求。
- `Prefill` 需要对齐到最长序列，浪费计算。
- `Prefill` 和 `Generation` 被迫共享同一份 topology 和 sharding。

更好的做法是：

- `Continuous batching`：让生成端持续接入新请求，减少空槽。
- `Disaggregated serving`：把 `Prefill` 和 `Generation` 分到不同服务器，分别优化。
- `Prefix caching`：复用重复前缀的 `KV cache`，尤其适合对话和 few-shot prompt。

JetStream 这类实现的关键思想就是把这三件事系统化：前缀缓存、连续批处理、prefill/generate 分离，并用控制器协调 KV 传输。

## 工程结论

推理优化的重点不是单纯追求更高 MFU，而是让不同阶段落在各自的瓶颈边界上：

- `Prefill` 倾向于吃满计算。
- `Generation` 倾向于压低 KV 读写和通信成本。
- `KV cache` 往往是生成阶段最重要的内存和带宽约束。
- 服务器拆分后，前缀缓存和调度策略会直接影响可扩展性。

因此，Transformer 推理的工程目标不是“把一个前向做快”，而是把 `Prefill`、`Generation`、缓存和并行方式一起放进同一个性能模型里。

## 补充算例

- 在一个 `L=64, D=4096, F=16384, N=32, K=8, H=256` 的模型里，参数量约为 `18.4B`，`int8` KV cache 的开销约为 `262kB/token`。
- 若在 TPU v5e `4x4` 上服务 `128k` 上下文，并且 KV cache 可以完全分片，batch 上限约为 `7`；若把 `K` 降到 `1`，batch 上限会提升到约 `56`。
- 这个例子说明，长上下文推理的 batch 上限通常先被 KV cache 约束，而不是先被参数量约束。

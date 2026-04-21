# LLaMA 3 TPU

## 模型账本

LLaMA 3-70B 的配置可以直接从公开 config 读出：

- `L = 80`
- `D = 8192`
- `F = 28672`
- `N = 64`
- `K = 8`
- `H = 128`
- `V = 128256`

按常用的 Transformer 参数估算公式，参数量分成三部分：

- FFW 参数约 `56.3B`
- attention 参数约 `12B`
- vocab 参数约 `2.1B`

总参数量约为 `70.4B`。其中真正决定规模的仍然是 MLP 的三块大权重，attention 只是在这个量级上提供次要修正。

## FLOPs 与时延

密集 Transformer 的训练 FLOPs 可以近似看成每个 token 约 `6` 倍参数量：

$$
6 \times 70B \approx 4.2 \times 10^{11}
$$

这意味着：

- 每个 token 的训练开销约 `4.2e11` FLOPs。
- `15T` tokens 对应总 FLOPs 约 `6.3e24`。

若在 `8960` 张 TPU v5p 上训练，并假设 `40% MFU`，则总训练时间约 `44` 天。这个数量级说明，LLaMA 3-70B 的主要约束已经不是“能不能算”，而是“能否持续把 TPU 喂饱”。

## 内存

在 bf16 参数、fp32 Adam 状态、并且每层只做有限 checkpoint 的前提下，4M token batch 的总内存约为 `21.6TB`。

其中：

- 参数和优化器状态约 `700GB` 量级。
- 激活 checkpoint 才是主要内存来源。

如果只看存储下限，约 `225` 张 TPU v5p 就能放下这套训练配置。但这并不意味着划算，因为这会把总训练时间拉得极长。训练大模型时，通常先受 FLOPs 约束，再受显存约束。

## 分片策略

如果不做序列切分，纯数据并行最多只能吃到 1024 条 sequence 的 batch 规模，因此无法直接把 4M token batch 放到更大的拓扑上。

放宽到序列/上下文也能切分后，纯 FSDP 仍然会通信受限。对 8960 芯片的 v5p pod，per-chip batch 约为 `468`，明显低于 FSDP 的通信阈值。

可行的方案是混合 FSDP 与 tensor parallelism：

- 纯 TP 的安全范围大约在 8-way 左右，16-way 开始吃紧。
- 混合方案的较优点大约是 `2048`-way FSDP 配 `4`-way TP。
- 这时每卡 batch 可以压到约 `100` token 量级，重新回到 compute-bound 区域。

工程上，这说明 LLaMA 3-70B 在 TPU 上并不是靠单一并行策略解决的，而是靠“数据并行 + 序列并行 + tensor parallelism”共同把 batch、权重和激活都切到合适粒度。

## 补充算例

- 按公开配置，LLaMA 3-70B 的参数可拆成 `56.3B` 的 FFW、`12B` 的 attention 和 `2.1B` 的词表，总计约 `70.4B`，主项仍然来自 MLP。
- 以 `15T` tokens 训练时，总 FLOPs 约为 `6.3e24`；若使用 `8960` 张 TPU v5p 且维持 `40% MFU`，总训练时间约 `44` 天。
- `4M` token batch 对应的总内存约为 `21.6TB`，其中激活 checkpoint 占主导；即使只从存储下限看，约 `225` 张 TPU v5p 也能放下，但这不代表训练时间可接受。
- 将 `4M` token batch 放到 `8960` 张芯片上时，每卡约 `468` token，纯 FSDP 仍会通信受限；更实用的切分仍是混合 FSDP + TP。

## 结论

- LLaMA 3-70B 的参数主项仍然是 FFW。
- 训练总 FLOPs 主要由 `6 × 参数数 × token 数` 决定。
- 40% MFU 下，8960 芯片训练 15T tokens 约需 44 天。
- 纯 DP 先卡在序列 batch，纯 FSDP 再卡在通信。
- 实用方案是混合 FSDP + TP，并把序列维也纳入切分。

# Transformer 数学

## 计数规则

Transformer 的 FLOPs 统计，核心是先区分 contracting 维和 batching 维。

- contracting 维是在运算中被求和消去的轴。
- batching 维在两个输入中同时出现，并原样保留到输出。

对于一般的张量 contraction，FLOPs 约等于所有 contracting 维和 batching 维乘积的两倍。若没有 contracting 维，则退化为逐元素乘法，因而不再额外乘 2。

对训练更重要的是反向传播。若 `C = A B`，则：

- 前向为 `2NPM` FLOPs。
- `dB` 需要 `2NPM` FLOPs。
- `dA` 也需要 `2NPM` FLOPs。

因此，一个矩阵乘法在训练中的总成本近似为 `6NPM` FLOPs，而推理只需要 `2NPM`。这也是 Transformer 训练常用 “每个 token 约 6 倍参数量 FLOPs” 估算的来源。

## Transformer 账本

把一层 Transformer 近似为 MLP 加 attention 后，可以得到一组稳定的一阶估算。

- MLP 参数量约为 `3DF`。
- attention 参数量约为 `4DNH`。
- 其他项如 layernorm 通常可忽略到一阶。

训练 FLOPs 方面：

- MLP 每层约为 `18BTDF`。
- attention 的 QKVO 投影约为 `24BTDNH`。
- dot-product attention 还要加上 `12BT^2NH`。

因此，dense Transformer 的总训练 FLOPs 常可写成：

$$
18BTDF + 24BTDNH + 12BT^2NH
$$

对多数中短上下文训练，`T < 8D` 时，MLP 仍是主项，attention 的二次项还不会主导总量。这也是为什么模型规模继续增大时，Transformer 仍然主要由矩阵乘法决定，而不是被 attention 的二次复杂度立即压垮。

## 稀疏与缓存

MoE 的一阶理解很直接：每层不是一个密集 MLP，而是 `E` 个专家；每个 token 只激活其中 `k` 个。

- 参数量按 `E` 放大。
- 激活计算按 `k` 计算。
- 通信会额外引入 token 路由前后的 `AllToAll`。

因此，MoE 的主要收益是参数容量增长快于激活计算；代价是路由和跨设备重排会把一部分收益换成通信复杂度。

Gradient checkpointing 的本质是用额外 FLOPs 换显存。只保存部分中间激活可以把存储压下去，但反向时需要重算前向子图，常见策略包括：

- 只保存每层输入，显存最省，但重算最多。
- 只保存大 matmul 的输出，折中重算量和存储量。

KV cache 则把推理阶段的代价转移到内存：

$$
2SLKH
$$

其中 `2` 表示 key 和 value。上下文越长、层数越深、KV heads 越多，缓存越大。GMQA 或 MQA 的价值主要就在于压低 `K`。

Flash Attention 的价值主要不在减少 FLOPs，而在避免显式 materialize 完整 attention matrix，从而把长上下文训练的主要压力从内存峰值和带宽搬运上移开。

## 补充算例

- 若 `D=4096`、`F=4D`、`V=32000`、`L=64`，模型总参数约为 `16B`，其中 attention 约占四分之一，参数主项仍然是 MLP。
- 在 int8 KV cache 且使用标准多头注意力时，单 token 的缓存约为 `512kB`，因此上下文长度和层数都会直接放大推理内存。
- 对 `A[I,J,K,L] * B[I,J,M,N,O] -> C[K,L,M,N,O]`，没有 batching 维，FLOPs 就是 `2IJKLMNO`，只有 contracting 维 `I` 和 `J` 被求和消去。
- 只保留 matmul 输出做 checkpoint 时，反向重算最贵的部分通常来自 attention 的两个 `T×T` 矩阵乘法，因此长上下文下保存激活的代价更高。

## 结论

- Transformer 的主账本可以近似看作矩阵乘法的账本。
- 对大多数密集模型，MLP 参数量和 FLOPs 都是主项。
- attention 的二次成本只有在足够长上下文下才会抬升到主导位置。
- MoE 主要换来更大的参数容量，但需要支付路由通信。
- checkpointing 和 KV cache 都是在“显存/带宽/重算”三者之间重新分配成本。

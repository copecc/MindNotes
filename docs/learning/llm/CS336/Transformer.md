---
title: Transformer 结构
---

# Transformer 结构

!!! abstract

    本文整理 decoder-only Transformer language model 的结构性笔记，聚焦整体前向路径、注意力机制、前馈网络、归一化与位置编码、工程实现边界，以及常见结构演变。

## Transformer 要解决什么问题

RNN 按时间步顺序更新隐藏状态，天然不利于长序列并行计算；当上下文变长时，早期信息还必须经过多步状态传递才能影响当前位置。路径变长会增加优化难度，也会让长程依赖更难稳定保留。Transformer 的核心设计，是让每个位置都能通过 attention 直接访问前文中与当前预测最相关的表示，同时保留训练阶段的并行矩阵计算能力。

对语言模型而言，目标是根据前文预测下一个 token。若序列为

$$
(x_1, x_2, \dots, x_T),
$$

则 decoder-only language model 建模的是条件概率分解

$$
p(x_1, x_2, \dots, x_T) = \prod_{t=1}^{T} p(x_t \mid x_{<t}).
$$

Transformer 要解决的，不只是“如何做一个更深的网络”，而是如何在保持自回归约束的前提下，高效表达长上下文中的依赖关系，并把计算组织成适合 GPU 的批量矩阵运算。

## 输入与输出

对 decoder-only Transformer LM 而言，主路径可以先按输入侧与输出侧来区分。输入侧先把 token ID 映射到 embedding 空间，形成 block stack 的初始 hidden states；这些表示随后依次通过多层 Transformer block；输出侧再把最后一层 hidden states 投影回词表，得到每个位置上的 next-token logits。若批大小为 $B$、序列长度为 $T$、模型宽度为 $d_{model}$，则主路径上的 hidden states 张量形状通常为

$$
X \in \mathbb{R}^{B \times T \times d_{model}}.
$$

???+ note "Embedding 与输出投影"

    ```python title="embedding_and_lm_head.py"
    --8<-- "learning/llm/CS336/transformer/embedding_and_lm_head.py"
    ```

`Embedding` 的作用是把 token ID 映射到连续向量空间。设词表大小为 $V$，embedding 矩阵为

$$
E \in \mathbb{R}^{V \times d_{model}},
$$

则输入 token 序列经过查表后先得到 block stack 的输入表示

$$
H^{(0)} = E[x],
$$

其中

$$
H^{(0)} \in \mathbb{R}^{B \times T \times d_{model}}.
$$

`input_hidden` 只是输入表示阶段的结果；它后面还会依次经过多层 attention、FFN、residual 和 norm，形成最终 hidden states $H$。输出侧再把这些最终表示投影回词表维度，得到 logits：

$$
\mathrm{logits} = H E^\top.
$$

它的含义其实是：对每个位置上的 hidden vector $h_t \in \mathbb{R}^{d_{model}}$，分别与词表里每个 token 的 embedding 向量做一次点积，得到该位置对整个词表的打分。若词表大小为 $V$，则

$$
h_t E^\top \in \mathbb{R}^{V}.
$$

其中第 $i$ 个分量就是

$$
(h_t E^\top)_i = h_t \cdot E_i,
$$

表示当前位置的表示 $h_t$ 与第 $i$ 个 token 向量 $E_i$ 的相似度。分数越高，说明模型越倾向于在这个位置预测该 token。

???+ example "输出投影示例"

    设词表里只有三个 token，它们的 embedding 为

    $$
    E =
    \begin{bmatrix}
    1 & 0 \\
    0 & 1 \\
    1 & 1
    \end{bmatrix},
    $$

    某个位置的最终 hidden state 为

    $$
    h_t = [2, 1].
    $$

    则

    $$
    h_t E^\top = [2, 1, 3].
    $$

    这表示模型对三个 token 的未归一化打分分别是 2、1、3，因此第三个 token 当前最可能。

很多语言模型会直接共享输入 embedding 与输出 projection 的权重，即 weight tying。这样既减少参数量，也让输入输出空间保持一致。对第 $t$ 个位置，训练时通常只取该位置的 logits 去预测下一个 token $x_{t+1}$。

## 注意力

attention 的输入是查询向量 $Q$、键向量 $K$ 与值向量 $V$。在 self-attention 中，它们来自同一段 hidden states 的不同线性投影。核心计算是

$$
\mathrm{Attention}(Q, K, V) = \mathrm{softmax}\left(\frac{QK^\top}{\sqrt{d_k}} + M\right)V.
$$

其中 $QK^\top$ 给出查询与各个键之间的相关性打分，$M$ 是 mask 项。对 decoder-only 模型，mask 必须保证位置 $t$ 只能看到 $\le t$ 的 token，因此通常使用 causal mask 在 softmax 之前屏蔽未来位置。

缩放因子 $1 / \sqrt{d_k}$ 的作用，是控制打分项的方差。若不缩放，随着 head 维度增大，$QK^\top$ 的数值范围会变大，softmax 更容易进入极端饱和区间，使梯度分布变差。在 `torch` 实现里，通常直接调用 `torch.softmax(scores, dim=-1)`；其内部会做稳定化处理。

attention 的结果可以理解为：当前位置不是只沿着固定深度路径传递信息，而是直接从所有可见位置中做一次加权聚合。权重由内容相关性动态决定，因此模型能根据当前 token 的表示，选择性读取前文中更有用的上下文片段。

### Causal Mask

自回归语言模型要求位置 $t$ 的表示不能依赖 $x_{t+1}, x_{t+2}, \dots$。因此 mask 的语义不是“可选优化”，而是模型定义的一部分。工程上通常不会把被屏蔽位置的 attention 权重直接乘零，而是在 softmax 前对相应位置加极小值，这样 softmax 后这些位置的概率才会稳定接近零。

???+ example "Causal Mask 示例"

    设某个 query 位置对四个 key 位置的原始打分为

    $$
    [2.0, 1.0, 3.0, 0.5].
    $$

    如果当前 query 来自第 2 个位置，那么它只能看第 1、2 个位置，后两个未来位置必须被屏蔽。此时可以把 mask 后的分数写成

    $$
    [2.0, 1.0, -\infty, -\infty].
    $$

    softmax 之后，概率只会分配给前两个位置；后两个位置的权重为 0。这样模型在定义上就不可能偷看未来 token。

### Softmax

softmax 的作用，是把每个查询位置对所有可见位置的打分归一化成概率分布。对向量 $z$，它的定义是

$$
\mathrm{softmax}(z_i) = \frac{e^{z_i}}{\sum_j e^{z_j}}.
$$

直接按这个公式实现时，若某些分量很大，$e^{z_i}$ 可能溢出。更稳定的写法是先减去当前行最大值 $m = \max_j z_j$：

$$
\mathrm{softmax}(z_i)
= \frac{e^{z_i - m}}{\sum_j e^{z_j - m}}.
$$

因为分子分母同时乘上了同一个 $e^{-m}$，结果不变，但指数项的数值范围会显著收缩。这就是常说的 safe softmax。

??? example "Safe Softmax 示例"

    设某个 query 位置对四个 key 位置的原始打分为$z = [1000, 1001, 999].$

    直接计算 $e^{z_i}$ 会导致数值溢出；而 safe softmax 先减去最大值 1001，得到$z - \max(z) = [-1, 0, -2],$

    再计算
    
    $$
    \mathrm{softmax}(z) = \frac{[e^{-1}, 1, e^{-2}]}{e^{-1} + 1 + e^{-2}}.
    $$

    此时数值稳定得多，同时最大分量对应的位置仍然拥有最高概率。

???+ note "Softmax"

    ```python title="softmax.py"
    --8<-- "learning/llm/CS336/transformer/softmax.py"
    ```

在 attention 中，softmax 是沿着未被 mask 屏蔽的位置这一维做归一化的。也就是说，对每个 query 位置，模型都会把它对所有 key 位置的打分转成一组和为 1 的权重。

??? hint "log-sum-exp"

    与 softmax 紧密相关的另一个表达是 log-sum-exp：

    $$
    \log \sum_j e^{z_j}.
    $$

    它经常出现在 log-softmax 与交叉熵中。若直接计算 $e^{z_j}$ 再求和取对数，同样会遇到溢出问题，因此稳定写法通常是令 $m = \max_j z_j$，再写成

    $$
    \log \sum_j e^{z_j}
    = m + \log \sum_j e^{z_j - m}.
    $$

    对语言模型输出 logits $z$ 而言，目标 token 为 $y$ 时，其负对数似然可以写成

    $$
    -\log p(y \mid x) = -z_y + \log \sum_j e^{z_j}.
    $$

    这里后一项就是 log-sum-exp。因此，safe softmax 与 log-sum-exp 本质上处理的是同一类数值稳定问题，只是前者更常出现在概率归一化，后者更常出现在 log-softmax、cross-entropy 和配分函数计算中。

### 位置编码

位置编码解决的是另一类问题：attention 本身只看 token 间的内容匹配，若完全不给位置线索，模型无法区分“相同 token 出现在不同顺序”这一事实。早期 Transformer 常用绝对位置编码，把位置向量直接加到 embedding 上。现代 decoder-only 模型更常使用 RoPE 或 ALiBi 这样的相对位置方案。

RoPE 的核心做法，是把偶数维和奇数维配对成二维子空间，然后按位置相关角度做旋转。设第 $m$ 个位置上的某一对二维分量为

$$
[x_{2i}, x_{2i+1}],
$$

对应角频率为 $\theta_i$，则在位置 $m$ 处的旋转可以写成

$$
\begin{bmatrix}
x'_{2i} \\
x'_{2i+1}
\end{bmatrix}
=
\begin{bmatrix}
\cos(m\theta_i) & -\sin(m\theta_i) \\
\sin(m\theta_i) & \cos(m\theta_i)
\end{bmatrix}
\begin{bmatrix}
x_{2i} \\
x_{2i+1}
\end{bmatrix}.
$$

因此，RoPE 本质上是在每个二维子空间中施加一个与绝对位置 $m$ 相关的旋转。更关键的是，当 query 位于位置 $m$、key 位于位置 $n$ 时，它们旋转后的内积会变成只与相对位置差 $m-n$ 相关的形式。这正是 RoPE 能把位置信息直接注入 attention 打分的原因：位置不再通过额外向量相加进入模型，而是直接改变 $QK^\top$ 的几何关系。更直接地说，RoPE 让 attention score 同时保留内容相似性，并对 token 之间的相对位移敏感。

从复数表示看，这个过程也可以理解为把每对二维分量写成复数 $x_{2i} + j x_{2i+1}$，再乘上一个位置相关相位因子 $e^{j m \theta_i}$。这种写法更紧凑，也更容易看出“旋转后内积依赖相对位移”的性质。

从实现角度看，RoPE 还需要先构造每个位置、每个二维频率对子对应的 $\cos(m\theta_i)$ 与 $\sin(m\theta_i)$。常见做法是先生成位置索引 $m$ 与频率序列 $\theta_i$，再用外积一次性得到所有角度，最后缓存对应的 cos/sin 表。这样推理时就不必为每个新 token 重新计算整段历史位置的三角函数。

工程上更常见的优化是：预先缓存 cos/sin 表、按设备和 dtype 复用、仅在上下文长度增长时扩展缓存，以及让它们与 `Q/K` 的 shape 对齐后直接 broadcast。`pin memory` 不是 RoPE 的核心优化点；它更偏向 CPU 到 GPU 数据传输场景，而 RoPE 的主要开销通常在设备内的三角函数计算、缓存管理与张量广播。

???+ note "RoPE"

    下面这段代码分成三步：先生成位置与频率对应关系，再构造可复用的 `cos/sin cache`，最后对每个二维子空间执行旋转。

    ```python title="rope.py"
    --8<-- "learning/llm/CS336/transformer/rope.py"
    ```

相比直接把位置向量加到 hidden states 上，RoPE 更贴近注意力打分机制本身，也更便于在长上下文设定下延展。ALiBi 则通过对 attention 分数加入距离偏置体现位置信息，形式更简单，但语义路径不同。

???+ example "RoPE 旋转示例"

    可以先看二维情形。若某个 query 的局部二维分量是

    $$
    [x_1, x_2] = [1, 0],
    $$

    位置对应的旋转角为 $\theta$，则旋转后得到

    $$
    [x_1 \cos\theta - x_2 \sin\theta,\ x_1 \sin\theta + x_2 \cos\theta]
    = [\cos\theta, \sin\theta].
    $$

    这意味着：原本沿第一个坐标轴方向的向量，被旋转到了角度为 $\theta$ 的新方向上。若位置从 $m=1$ 变到 $m=2$，对应角度从 $\theta$ 变到 $2\theta$，则同一个二维分量会进一步旋转为

    $$
    [\cos 2\theta, \sin 2\theta].
    $$

    因此，位置变化并不是简单地给向量“加一个位置偏移”，而是在每个二维子空间里直接改变它的方向。

    再看 query 和 key 的相对位置。若 query 位于位置 $m$、key 位于位置 $n$，并对它们施加同一频率下的旋转，则 attention score 中对应的二维内积会依赖角度差

    $$
    (m-n)\theta,
    $$

    而不是分别单独依赖 $m$ 和 $n$。因此，当两个 token 的相对位移保持不变时，这一部分由 RoPE 注入的位置关系也保持一致。这正是它适合用于 attention 打分的原因。

    更高维时，RoPE 只是把整个向量拆成许多这样的二维对子，分别用不同频率做同类旋转。因此它不是给 hidden state 额外拼一个位置向量，而是直接改变 `Q/K` 在各二维子空间中的方向。

现代 decoder-only 模型通常只在 attention 前对 $Q$ 与 $K$ 施加 RoPE，而不作用于 $V$。原因是 attention 打分依赖 $QK^\top$；只要位置信息进入查询和键，权重分配就能感知相对位置。若把 RoPE 也作用到 $V$，则会改变被聚合内容本身，语义上并不必要。KV cache 只会改变推理时 attention 的计算路径，而不会改变模型的语义表示。

### Multi-Head Attention

若只使用单头 attention，模型在每一层只能产生一组上下文混合结果。multi-head attention 的做法，是把通道维拆成多个 head，在每个子空间中分别学习不同的匹配模式，再把各头结果拼接回去做输出投影。若 head 数为 $h$，则常见约定是

$$
d_{head} = d_{model} / h.
$$

???+ note "MHA 实现"

    ```python title="mha.py"
    --8<-- "learning/llm/CS336/transformer/mha.py"
    ```

这里的输入张量通常写成

$$
x \in \mathbb{R}^{B \times T \times d_{model}},
$$

也就是 `(batch_size, seq_len, d_model)`。在标准实现中，还需要满足 `d_model % num_heads == 0`，这样每个 attention head 才能分到固定宽度的 `head_dim`。主路径可以直接按张量流来读：先由输入 $x$ 线性投影得到 `Q/K/V`，再拆成多头，对 `Q/K` 施加 RoPE，然后计算打分、加上 causal mask、做 softmax，最后对 `V` 加权求和、并头并经过输出投影。

工程实现里还常把 `W_Q / W_K / W_V` 融合成一个更大的 `W_{qkv}`，先一次矩阵乘法得到拼接后的 `QKV`，再切分成三段。这样做不会改变注意力的数学定义，但通常能减少投影阶段的 kernel 启动与访存开销。

多头机制的意义不只是“并行算很多次 attention”。更重要的是，不同 head 可以在不同表示子空间中学习不同类型的关系，例如局部语法关系、远距离指代或结构边界。输出投影再把这些子空间的结果重新混合回统一的模型空间。

???+ example "Multi-Head Shape 示例"

    设输入张量形状为

    $$
    (B, T, d_{model}) = (2, 4, 8),
    $$

    并取头数 $h = 2$，则每个 head 的宽度为

    $$
    d_{head} = 8 / 2 = 4.
    $$

    此时拆头前的形状是 `(2, 4, 8)`，拆头后变成 `(2, 2, 4, 4)`，分别对应 `(batch, heads, seq, head_dim)`。注意这里并没有改变 token 数量，只是把最后一个通道维重新分组。

### 注意力变体

#### MHA

MHA（Multi-Head Attention）为每个 query head 配置独立的 key/value heads，因此不同 head 可以在各自子空间中学习相对独立的上下文读取模式。它的优势是表达能力最完整，模型可以同时保留多种关系模式；代价则是参数、KV cache 与访存开销都相对更高。对实现而言，MHA 的基本约束是 `d_model % num_heads == 0`，因为只有这样才能把模型维度均匀拆分到所有 heads 上。

#### MQA

MQA（Multi-Query Attention）让所有 query heads 共享同一组 key/value heads。这样会显著减少推理阶段的 KV cache 容量，并降低长上下文下的带宽压力，因此在强调推理效率的系统中较常见。相应代价是，各个 query heads 不再拥有完全独立的 key/value 表示基础，能够学习的读取模式会受到更多共享约束。

#### GQA

GQA（Grouped-Query Attention）把 query heads 分成若干组，每组共享一组 key/value heads，在表达能力与推理成本之间做折中。它保留了比 MQA 更强的多样性，同时又比 MHA 使用更少的 KV heads，因此往往能在质量与效率之间取得更平衡的结果。

???+ note "GQA 实现"

    ```python title="gqa.py"
    --8<-- "learning/llm/CS336/transformer/gqa.py"
    ```

从实现角度看，GQA 和 MHA 的 attention 主公式并没有变化，差异主要发生在 `Q/K/V` 的头组织方式上。典型约束包括：`d_model % num_q_heads == 0`，以及 `num_q_heads % num_kv_heads == 0`。前者保证 query heads 可以均匀拆分；后者保证较少的 `K/V heads` 可以按组对齐到全部 query heads。代码里常见的 `expand`、`repeat` 或 grouped broadcast，本质上都在完成这一对齐过程。

#### MLA

MLA 一类方法进一步压缩或重参数化注意力内部表示，目标通常是继续降低长上下文推理时的显存占用、带宽消耗或 cache 成本。它关注的不再只是“共享多少个 KV heads”，而是直接改变注意力内部状态的参数化与存储方式。因此，MLA 更适合放在结构演进与系统优化语境下理解，而不宜简单视为 MHA、MQA、GQA 的同层级小改动。

总体来看，这几类变体都保持了 attention 的基本计算骨架，即“相关性打分 → mask → softmax → 对 `V` 加权求和”。真正发生变化的是 `Q/K/V` 的参数化方式、head 的共享关系，以及推理阶段的内存访问模式与缓存成本。

!!! hint "注意力变体总结"

    前面讨论的 attention 主公式并不因这些变体而改变。变化主要发生在 `Q/K/V` 的参数化方式、head 的共享关系，以及推理阶段的 KV cache 与带宽成本上。

    | 变体 | Query heads    | KV heads         | 共享方式                                           | 优点                                    | 代价                                     |
    | ---- | -------------- | ---------------- | -------------------------------------------------- | --------------------------------------- | ---------------------------------------- |
    | MHA  | 多             | 多               | 每个 query head 对应独立的 key/value head          | 表达能力最完整                          | 参数、KV cache 与访存开销更高            |
    | MQA  | 多             | 1                | 所有 query heads 共享同一组 key/value              | KV cache 最省，长上下文推理带宽压力最低 | 不同 query heads 的读取模式共享约束更强  |
    | GQA  | 多             | 少于 query heads | 多个 query heads 分组共享 key/value                | 在质量与推理成本之间折中                | 表达能力通常仍弱于完全独立的 MHA         |
    | MLA  | 视具体设计而定 | 视具体设计而定   | 不只是 head 共享，还会进一步压缩或重参数化内部状态 | 可以继续降低长上下文推理成本            | 结构更复杂，不能只用 head 共享关系来理解 |


## 前馈网络

attention 负责跨位置的信息混合，前馈网络则在每个位置上独立地做通道变换。它不改变序列长度，却显著贡献参数量与计算量。若把 attention 看成“在 token 之间路由信息”，那么前馈网络更接近“在每个位置内部重组和变换特征”。

最基础的 position-wise FFN 可写成

$$
\mathrm{FFN}(x) = W_2 \sigma(W_1 x),
$$

其中 $W_1$ 把维度从 $d_{model}$ 升到更大的 $d_{ff}$，$W_2$ 再投回模型维度。现代大语言模型里更常见的是 GLU 系列门控变体，例如 SwiGLU：

$$
\mathrm{SwiGLU}(x) = W_{down}\bigl(\mathrm{SiLU}(W_{gate}x) \odot (W_{up}x)\bigr).
$$

这里的 SiLU 是

$$
\mathrm{SiLU}(x) = x \cdot \sigma(x) = \frac{x}{1 + e^{-x}},
$$

也常被称为 Swish。它先用 sigmoid 生成一个随输入变化的平滑门，再用这个门去调制原始输入，因此比纯 ReLU 更连续。这样一来，SwiGLU 实际上是在用 $SiLU(W_{gate}x)$ 控制 $W_{up}x$ 中哪些通道应被放大、保留或抑制。

???+ note "FFN 与 SwiGLU"

    ```python title="ffn_swiglu.py"
    --8<-- "learning/llm/CS336/transformer/ffn_swiglu.py"
    ```

SwiGLU 相比普通两层 MLP，多了一条门控路径。它允许模型按输入内容动态调制哪些通道应被放大或抑制，因此在相近参数预算下往往有更好的表达能力。工程上，FFN 虽然不做跨 token 混合，但在大模型中通常占据很大一部分参数量和 FLOPs，因此绝不是次要模块。

Transformer 不只是把 attention 与 FFN 顺序拼接起来。为了让深层网络在训练时保持稳定，还需要在子层前后安排归一化与残差路径。现代大语言模型里，这一部分通常体现为 pre-norm 结构，而最常见的归一化选择则是 RMSNorm。

## Transformer Block

RMSNorm 与 LayerNorm 一样都用于稳定训练，但 RMSNorm 只按均方根做缩放，不再减均值。若输入为 $x \in \mathbb{R}^{d}$，则 RMSNorm 可写为

$$
\mathrm{RMSNorm}(x) = \gamma \odot \frac{x}{\sqrt{\frac{1}{d}\sum_{i=1}^{d} x_i^2 + \varepsilon}}.
$$

???+ note "RMSNorm"

    ```python title="rmsnorm.py"
    --8<-- "learning/llm/CS336/transformer/rmsnorm.py"
    ```

与 LayerNorm 相比，RMSNorm 少了减均值这一步，结构更简单，也更符合很多现代大模型的实现习惯。它的目标不是改变表示语义，而是控制不同层之间的数值尺度，使深层网络更容易优化。相比之下，BatchNorm 依赖 batch 维统计量，更适合卷积网络等样本间统计较稳定的场景；而 Transformer 更常对每个位置的最后一维表示单独做归一化，因此 LayerNorm / RMSNorm 更自然。

pre-norm block 的典型顺序是 norm → attention → residual → norm → FFN → residual。多层 block 级联之后，再接 final norm 与 LM head，就得到完整的 decoder-only Transformer LM。

???+ note "Pre-Norm Block"

    ```python title="transformer_block.py"
    --8<-- "learning/llm/CS336/transformer/transformer_block.py"
    ```

pre-norm 的关键在于：进入 attention 和 FFN 之前先做归一化，而残差分支保持原始主路径。与 post-norm 相比，这种结构通常更利于深层训练，因为残差路径上的信息传播更直接，梯度在深层网络中的退化也更轻。现代大语言模型大多采用 pre-norm 变体，而不是原始 Transformer 论文中的 post-norm 结构。

从单个 block 的视角看，attention 子层负责跨位置聚合信息，FFN 子层负责逐位置变换通道表示，残差则把这两种更新稳定地叠加回主路径。整层的输出仍然保持与输入相同的张量形状，因此多层 block 可以直接堆叠。

## 语言模型前向过程

把各模块串起来后，decoder-only Transformer LM 的整体前向路径可以概括为：token embedding → 多层 Transformer block → final norm → output projection。

???+ note "Transformer LM 前向"

    ```python title="transformer_lm.py"
    --8<-- "learning/llm/CS336/transformer/transformer_lm.py"
    ```

训练时，整段序列通常一次性送入模型，并借助 causal mask 保证每个位置只能看到前文，因此所有位置的前向计算可以并行完成。推理时则不同：模型每生成一个新 token，都要把它追加到上下文末尾，再计算下一步的 logits。训练和推理的数学定义一致，但计算组织方式不同，这也是 KV cache 会出现在推理实现中的原因。

## 工程实现

Transformer 的数学定义相对简洁，但真实实现很容易在 shape、mask、cache、数值稳定性和内存访问模式上出错。

首先是 shape 约定。隐藏状态常见形状为 `(batch, seq, d_model)`，拆头后变为 `(batch, heads, seq, head_dim)`。很多 bug 并不来自公式，而是来自 reshape、transpose 和广播维度用错。只要 `Q / K / V` 的 head 维、序列维和批维顺序错位，attention 权重的语义就会整体偏移。

其次是 mask 的施加位置。mask 必须在 softmax 之前进入分数矩阵；若在 softmax 之后再乘零，剩余位置的概率和不再自动归一，也容易引入数值与语义错误。对于较长上下文，softmax 前减去每行最大值几乎是标准做法。

推理时的 KV cache 也是一个高频出错点。其语义不是“缓存所有中间结果”，而是缓存每一层历史 token 对应的 $K$ 和 $V$，使新 token 到来时不必重算整段前缀。

???+ note "KV Cache"

    ```python title="kv_cache.py"
    --8<-- "learning/llm/CS336/transformer/kv_cache.py"
    ```

这里需要特别注意追加的维度应是时间维，而不是 head 维或 batch 维。训练阶段通常不使用 cache，因为整段序列本来就并行计算；cache 主要改变的是自回归推理的计算代价，而不是模型定义。

???+ example "KV Cache 示例"

    设某一层当前缓存的 key 张量形状为 `(B, H, T, D)`，例如 `(1, 8, 16, 64)`，表示已经缓存了 16 个历史 token。现在新生成了 1 个 token，对应的新 key 张量形状就是 `(1, 8, 1, 64)`。

    这时正确的做法是沿时间维拼接，得到新的 cache 形状

    $$
    (1, 8, 17, 64).
    $$

    若错误地沿 head 维或 batch 维拼接，张量虽然可能还能算，但语义已经完全错了：模型不再是在“历史序列后面追加一个新位置”，而是在伪造新的头或新的样本。

更进一步，FlashAttention、PagedAttention 等优化关注的是访存模式、kernel 融合与 cache 管理。它们提升的是工程效率，而不是改变 attention 的数学形式。理解这一点很重要：优化实现时，应区分“算法定义”和“内核调度”。

## 复杂度与规模估算

在序列长度为 $T$、模型宽度为 $d$ 时，attention 的主要代价随 $T^2$ 增长，而 MLP 的主要代价更接近 $T \cdot d_{ff}$。这意味着：当上下文较短时，FFN 往往占据更多计算；当上下文继续增长时，attention 的二次项会逐渐成为瓶颈。

???+ note "复杂度估算"

    ```python title="flops_estimate.py"
    --8<-- "learning/llm/CS336/transformer/flops_estimate.py"
    ```

参数量方面，embedding、attention 投影、FFN、norm 和输出投影共同构成模型规模。其中 FFN 往往是大头之一，而 KV cache 的内存开销则主要出现在推理阶段，并随层数、上下文长度和键值头数线性增长。

因此，讨论 Transformer 的“复杂度”时，至少要区分三件事：训练时的 FLOPs、推理时的单步代价，以及长上下文下的 cache 内存占用。它们相关，但不是同一个量。

## 演变与变体

原始 Transformer 同时包含 encoder 与 decoder，而现代大语言模型常采用 decoder-only 结构。在这一主线上，norm、位置编码、attention 头共享方式和 FFN 结构都发生了持续演变。

结构上，encoder-decoder Transformer 更适合机器翻译或条件生成，因为它需要单独编码输入，再由 decoder 结合 cross-attention 生成输出。decoder-only 结构则更适合统一语言建模接口：给定前缀，直接预测后续 token。对于大规模预训练语言模型，这种形式更直接，也更容易扩展数据与任务范围。

组件上，LayerNorm 常被 RMSNorm 取代；absolute positional encoding 常被 RoPE 或 ALiBi 取代；标准 multi-head attention 也在推理效率驱动下演化出多种键值共享方式。

前馈层方面，普通 FFN 逐渐被 GLU/SwiGLU 等门控变体替代。

这些变体的共同特点是：主体框架没有改变，仍然是 embedding、attention、FFN、residual、norm 的堆叠；变化主要发生在模块内部的参数化方式、位置信息注入方式，以及训练/推理成本的权衡上。

???+ tip "实现提示"

    检查 Transformer 实现时，通常至少要确认以下几点。

    - attention mask 是否在 softmax 前施加，并严格屏蔽未来位置。
    - `Q / K / V` 的张量形状、拆头顺序和并头顺序是否一致。
    - 缩放因子是否使用 $1 / \sqrt{d_k}$，softmax 是否做了数值稳定处理。
    - RoPE 是否只作用于 `Q / K`，而不是同时作用于 `V`。
    - pre-norm / post-norm 的结构位置是否与实现目标一致。
    - KV cache 是否沿时间维追加，而不是误拼到其他维度。
    - 训练路径与推理路径是否共享相同的数学定义，只在计算组织方式上不同。

    若这些条件不满足，即使模型能跑通，attention 权重语义、长上下文行为或推理效率也可能已经偏离预期。

???+ note "实现链接"

    见 [CS336 assignment1-basics](https://github.com/copecc/CS336/tree/main/assignment1-basics){target=_blank}。
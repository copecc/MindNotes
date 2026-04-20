---
title: KV Cache
---

# KV Cache

!!! abstract

    本文整理 decoder-only Transformer 推理中的 KV cache 机制，重点说明它解决的问题、prefill 与 decode 的阶段差异、缓存内容、复杂度变化、显存代价，以及围绕 KV cache 展开的主流工程优化与调度设计。目标不是记住若干孤立术语，而是建立一张完整的推理系统地图：KV cache 为什么出现、为什么会成为瓶颈、系统又如何围绕它组织内存与调度。

## KV Cache 的作用

在 decoder-only Transformer 的自回归生成中，模型每生成一个新 token，都要基于“历史上下文 + 当前 token”再次执行注意力计算。若不使用 KV cache，那么历史 token 对应的 key 和 value 会在后续每一步中被重复投影与重复读取。

以长度为 $T$ 的上下文为例。生成第 1 个 token 时，需要基于已有上下文执行一轮 attention；生成第 2 个 token 时，历史上下文再次参与 attention；生成第 3 个 token 时，前面已经处理过的历史位置又会再次参与。若每一轮都重新从隐藏状态投影出整段历史的 K 和 V，那么这些历史表示会被反复计算。

KV cache 的作用，是将历史位置已经计算完成的 key 与 value 按层保存下来。后续 decode 时，系统只需要为新增 token 计算当前步的 query、key、value，再让新的 query 与历史缓存中的 key/value 交互。

因此，KV cache 改变的是推理时 attention 的执行路径，而不是模型参数、训练目标或注意力公式本身。

???+ info "重点"

    KV cache 消除的是历史 key/value 的重复生成，而不是当前 query 对历史上下文的访问。随着上下文增长，decode 的访问成本仍然会上升。

## K 与 V 的缓存性

在自回归生成中，历史位置的 key 和 value 一旦生成，后续通常不会改变；但 query 则只服务于“当前这一步”发起的注意力查询。

更具体地说，当前步 attention 的基本形式可以写为：

$$
\text{Attention}(Q_t, K_{1:t}, V_{1:t}).
$$

其中 $Q_t$ 对应当前 token 的查询向量，$K_{1:t}$ 和 $V_{1:t}$ 对应从历史到当前位置的键和值。对后续第 $t+1$ 步而言，$Q_t$ 不再参与新的 attention 查询；它已经完成了自己的职责。相比之下，第 $t$ 步生成的 $K_t$ 和 $V_t$ 会继续被后续所有位置访问，因此它们具有跨时间步的复用价值。

这也是为什么系统通常缓存 K 和 V，而不缓存 Q：

- Q 只在当前步使用一次
- K 和 V 会被未来多个时间步反复读取
- 缓存 Q 不能显著减少后续计算，但会增加额外存储

从存储价值看，KV cache 关注的是“未来仍然会被读取的历史状态”，而不是“当前步已经消费完成的临时结果”。

## Prefill 与 Decode

推理通常分为 prefill 和 decode 两个阶段。

### Prefill

prefill 处理完整输入 prompt。系统需要对整段上下文执行一次完整前向，生成首批 KV cache，并得到首个可用于继续生成的状态。

这一阶段通常以大规模矩阵乘法为主，算子形态更接近 GEMM，数据复用较高，因此更容易表现为 compute-bound。若请求中带有很长的系统提示词、历史对话或 few-shot 示例，prefill 的一次性计算成本会明显上升。

从系统输入输出看，prefill 的核心特征有两点：

- 输入是一整段已有上下文
- 输出既包括当前生成所需的状态，也包括后续 decode 将持续使用的首批 KV cache

### Decode

decode 阶段每次只新增一个 token。由于历史 key/value 已经缓存，系统不再重算整段上下文，而是将当前 token 追加到已有缓存之后，再执行当前步 attention。

这一阶段的数据复用明显下降，系统往往需要频繁从显存中读取历史 KV，因此更容易表现为 memory-bound。随着生成长度变长，decode 的瓶颈通常不再是单次算子理论算力，而是显存带宽、缓存布局和调度效率。

从输入输出角度看，decode 的关键特征是：

- 输入通常是“上一步生成结果 + 历史 KV cache”
- 输出是新 token 以及更新后的 cache 尾部
- 单步计算量不大，但历史读取持续增长

### 阶段差异

prefill 与 decode 虽然都属于同一次推理请求，但资源画像并不相同。

- prefill 更偏向算力利用率
- decode 更偏向显存带宽与缓存访问效率
- 二者对批处理、调度和硬件资源的最优配置并不一致

这也是为什么高性能推理系统通常不会把 prefill 与 decode 当作完全同质的负载来处理。

???+ info "重点"

    prefill 与 decode 的差异不只是“一个处理整段输入，一个逐 token 生成”，更重要的是二者对应不同的系统瓶颈：前者更容易受算力限制，后者更容易受显存带宽限制。

## KV Cache 保存什么

KV cache 保存的是每一层 attention 在历史位置上已经计算完成的 key 与 value 张量，而不是整层前向中的全部中间结果。

原因在于，自回归生成时，历史 token 的 key/value 一旦生成，在后续步骤中通常不会再改变；真正需要持续参与后续计算的，是这些历史位置的表示能否被新的 query 读取。因此，具有复用价值的是按层保存的 K 与 V，而不是完整前向路径上的全部激活值。

从系统角度看，KV cache 更接近“为后续 attention 查询准备的只读历史状态”，而不是训练阶段用于反向传播的全量中间状态。

### 典型张量形状

不同实现的维度顺序可能略有不同，但从概念上看，每层 KV cache 通常至少包含以下维度：

- batch size
- 序列长度
- KV heads 数量
- head dimension

若使用多层 Transformer，则每一层都会维护自己的 K/V 缓存。因而总缓存规模并不是某一层的代价，而是所有层叠加后的总量。

### MHA、MQA、GQA 与 KV Cache

KV cache 的大小不仅和序列长度有关，也和注意力头的组织方式有关。

- **MHA（Multi-Head Attention）**：每个 query head 通常对应独立的 key/value head，KV cache 规模较大
- **MQA（Multi-Query Attention）**：多个 query head 共享同一组 key/value head，KV cache 明显更小
- **GQA（Grouped-Query Attention）**：介于二者之间，多个 query head 共享一组 KV heads，在显存占用与表达能力之间折中

因此，MQA 和 GQA 不只是注意力结构变化，也是在直接影响推理阶段的 KV cache 体积与带宽压力。

## 执行流程

从一次请求进入系统到连续生成 token，KV cache 的执行流程通常可以概括为三步：

1. prefill 阶段对整段 prompt 执行前向，生成各层的初始 KV cache
2. decode 阶段每步只为当前 token 计算新的 K/V，并将其追加到缓存尾部
3. 当前步 query 读取历史 KV cache，完成 attention 后生成下一个 token

若用极简伪代码表示，一个请求的 decode 主循环通常类似下面的结构。

???+ note "基础 KV Cache 流程示例"

    ```python title="basic_kv_flow.py"
    import torch


    class SimpleKVCache:
        def __init__(self, n_layers, n_kv_heads, max_seq_len, head_dim, device="cuda"):
            self.max_seq_len = max_seq_len
            self.cursor = 0
            self.key = [
                torch.empty(n_kv_heads, max_seq_len, head_dim, device=device)
                for _ in range(n_layers)
            ]
            self.value = [
                torch.empty(n_kv_heads, max_seq_len, head_dim, device=device)
                for _ in range(n_layers)
            ]

        def append(self, layer_idx, k_t, v_t):
            t = self.cursor
            self.key[layer_idx][:, t] = k_t
            self.value[layer_idx][:, t] = v_t

        def get_prefix(self, layer_idx):
            return (
                self.key[layer_idx][:, : self.cursor],
                self.value[layer_idx][:, : self.cursor],
            )

        def step(self):
            self.cursor += 1
            if self.cursor > self.max_seq_len:
                raise RuntimeError("KV cache overflow")
    ```

这个结构不是生产实现，但它准确表达了 KV cache 的核心：历史 K/V 只生成一次，之后持续被读取；每一步只向尾部追加当前 token 的 K/V。

## 复杂度与性能变化

若不使用 KV cache，自回归生成到第 $t$ 步时，历史位置对应的 key/value 会在每一步中反复投影，重复计算会随着上下文长度增长而快速累积。

使用 KV cache 后，历史 key/value 只在首次出现时计算一次。后续每一步主要新增三类开销：

- 当前 token 的投影计算
- 当前 query 对历史 KV cache 的读取与 attention 计算
- 新生成 key/value 的追加写入

因此，KV cache 会显著降低重复投影成本，但不会把 decode 变成常数代价。上下文越长，当前 query 需要访问的历史缓存仍然越多，attention 的读取与归约成本不会消失。

从工程效果上看，KV cache 最显著的收益通常体现在两点：

- 同样上下文下，单位 token 的重复计算明显减少
- 系统可以把更多资源集中在当前步 attention 和缓存管理上

### 非常数复杂度

一个常见误解是：既然历史 K/V 已经缓存，后续 decode 就会变成常数时间。这种说法并不准确。

KV cache 消除的是“历史位置重复投影”的代价，但当前 query 仍然要与历史位置的所有 K 做匹配，并对对应的 V 做加权聚合。也就是说，随着上下文变长，当前步访问的缓存长度仍然在增长。

因此，KV cache 减少的是重复生成成本，而不是把长上下文 attention 的读取与归约代价完全抹掉。

## 显存压力来源

KV cache 的主要代价是显存占用会随着层数、batch size、序列长度、头数和 head dimension 增长。对于长上下文、高并发推理系统，KV cache 往往比单次算子本身更早成为系统瓶颈。

从近似关系上看，单请求的 KV cache 开销可理解为与以下因素线性相关：

- 层数
- 序列长度
- key/value 头数
- 每个头的维度
- 数据类型字节数

若忽略常数项，单请求总缓存规模可粗略写为：

$$
\text{KV Cache Size} \propto L \times T \times H_{kv} \times D \times 2 \times \text{bytes},
$$

其中 $L$ 表示层数，$T$ 表示序列长度，$H_{kv}$ 表示 KV heads 数量，$D$ 表示每个头的维度，额外的 $2$ 对应 K 和 V 两份缓存。

这意味着上下文翻倍、并发翻倍或缓存精度提高，都会直接推高显存消耗。

因此，推理系统需要持续在几类目标之间做权衡：

- 保留更长上下文，以换取更强的上下文建模能力
- 支持更高并发，以提高设备吞吐
- 控制单请求缓存体积，以避免显存过早耗尽

在很多实际系统中，KV cache 管理已经不再是一个局部实现细节，而是决定最大上下文长度、并发规模和尾延迟的重要因素。

???+ info "重点"

    长上下文会拉长单请求缓存，高并发会增加同时驻留请求数，二者叠加后，KV cache 往往比权重更早成为推理显存瓶颈。

## 主流工程优化

围绕 KV cache 的工程优化，通常服务于三个目标：

- 提高显存利用率
- 减少重复计算或重复存储
- 降低 decode 阶段的显存带宽压力

### 静态连续分配（Static / Contiguous KV Cache）

较早期的实现通常会为每个请求预留一段连续显存，用于保存从起始位置到最大序列长度的全部 KV 空间。它的优点是地址计算简单、访问路径直接，单请求内部的数据布局也容易处理。

问题在于，这种方式通常需要提前按最大长度预留空间。若请求实际长度明显短于预留上限，就会形成内部碎片；不同请求频繁申请和释放连续大块空间时，也容易形成外部碎片。

因此，连续分配的优势是实现简单，代价是显存利用率较低，在高并发和长上下文场景下扩展性通常较差。

### PagedAttention

`PagedAttention` 的核心思想，是不再要求一个请求的 KV cache 在物理显存中连续存放，而是把缓存切分为固定大小的物理块，再通过 block table 维护逻辑位置到物理块的映射。

它解决的核心问题，是连续大块分配带来的显存碎片。逻辑序列仍然保持连续，但物理上可以按需扩展和回收，从而显著提升显存利用率。对推理系统而言，这意味着请求不需要一开始就为最大长度锁死整段显存，而可以随着生成逐块扩张。

它的代价在于地址管理更复杂，decode kernel 需要根据 block table 访问分散的物理块。换言之，这类方案用更复杂的缓存管理与 kernel 访问路径，换取更高的显存利用率与更好的并发承载能力。

在工程上，`PagedAttention` 适合下列场景：

- 请求长度差异较大
- 并发请求数量较多
- 系统希望尽量减少显存碎片和预留浪费

???+ info "重点"

    `PagedAttention` 改变的是缓存分配和访存组织方式，而不是 attention 的数学定义。它的关键收益来自内存管理，而不是公式层面的简化。

???+ note "PagedAttention 的 Torch 化示例"

    ```python title="paged_kv_cache.py"
    import torch


    class BlockPool:
        def __init__(self, num_blocks, block_size, n_kv_heads, head_dim, device="cuda"):
            self.block_size = block_size
            self.free_blocks = list(range(num_blocks))
            self.key_blocks = torch.empty(
                num_blocks, n_kv_heads, block_size, head_dim, device=device
            )
            self.value_blocks = torch.empty(
                num_blocks, n_kv_heads, block_size, head_dim, device=device
            )

        def alloc(self):
            if not self.free_blocks:
                raise RuntimeError("No free KV blocks")
            return self.free_blocks.pop()

        def free(self, block_id):
            self.free_blocks.append(block_id)


    class PagedRequestCache:
        def __init__(self, pool):
            self.pool = pool
            self.block_table = []
            self.seq_len = 0

        def append(self, k_t, v_t):
            logical_block = self.seq_len // self.pool.block_size
            offset = self.seq_len % self.pool.block_size

            if logical_block == len(self.block_table):
                self.block_table.append(self.pool.alloc())

            physical_block = self.block_table[logical_block]
            self.pool.key_blocks[physical_block, :, offset] = k_t
            self.pool.value_blocks[physical_block, :, offset] = v_t
            self.seq_len += 1

        def gather_prefix(self):
            ks = []
            vs = []
            remain = self.seq_len
            for block_id in self.block_table:
                take = min(remain, self.pool.block_size)
                ks.append(self.pool.key_blocks[block_id, :, :take])
                vs.append(self.pool.value_blocks[block_id, :, :take])
                remain -= take
            return torch.cat(ks, dim=1), torch.cat(vs, dim=1)
    ```

这个示例说明两点：

- 逻辑序列可以连续，但物理存储不必连续
- 系统真正维护的是 block table，而不是“大数组必须整段连续”的假设

### RadixAttention

`RadixAttention` 的重点不在单请求内部的分页，而在多请求之间的前缀共享。若多个请求具有相同的 prompt 前缀，系统就可以让它们复用同一组历史 KV blocks，而不是为每个请求重复保存一份。

这种机制解决的是两类冗余：

- 相同前缀对应的重复 KV 存储
- 相同前缀对应的重复 prefill 计算

它尤其适合系统提示词、固定 few-shot 示例、多轮对话共享历史等场景。此时，多请求之间真正共享的不是“文本字符串”，而是这些文本经过前向计算后生成的历史 KV 表示。

#### 命中条件

`RadixAttention` 能否生效，取决于请求之间是否存在足够长且可复用的公共前缀。只有当 token 序列前缀完全一致，且这段前缀已经被某个请求计算过并保存在缓存中时，后续请求才能直接命中已有前缀。

因此，它的收益并不来自“文本看起来相似”，而来自 token 级前缀的严格一致。系统提示词、模板化 few-shot、固定工具描述这类内容更容易产生稳定命中；开放式、多分支、用户输入差异很大的对话前缀，则命中率通常较低。

#### 共享粒度

从工程上看，系统共享的通常不是整个请求对象，而是前缀对应的一组 KV blocks。也就是说，`RadixAttention` 的共享粒度更接近“块级历史表示”，而不是“字符串级复用”。

这样做有两个原因：

- 真正需要复用的是已经生成好的 KV 表示，而不是原始文本
- 块级组织更容易与分页式缓存管理结合，便于后续请求在命中公共前缀后继续追加自己的私有尾部

因此，`RadixAttention` 更像是“共享前缀的缓存索引结构”，而不是一种独立于缓存块管理之外的单独机制。

#### 生命周期管理

它的主要代价在于共享关系管理会更复杂。系统需要维护引用关系、生命周期和回收逻辑，否则共享块容易出现错误释放、共享失效或过度保留。

在真实系统中，通常至少要回答几个问题：

- 当前这组共享 blocks 正被多少请求引用
- 某个请求结束后，哪些 blocks 还能继续保留
- 当请求在共享前缀之后继续生成时，新增尾部如何与共享前缀拼接
- 当前缀树增长很快但命中率下降时，哪些节点应被淘汰

这意味着 `RadixAttention` 的难点并不主要在 attention 公式，而在缓存索引、引用计数和回收策略。

#### 与 PagedAttention 的关系

从设计思路看，`RadixAttention` 和 `PagedAttention` 并不冲突：前者强调跨请求的前缀复用，后者强调块式内存管理；实际系统中二者可以结合使用。

可以把二者的关系理解为：

- `PagedAttention` 解决“一个请求的 KV cache 如何按块组织”
- `RadixAttention` 解决“多个请求如何共享这些块”

也就是说，`PagedAttention` 提供了更细粒度的物理存储单元，`RadixAttention` 则在这些块之上建立跨请求的复用索引。

#### 适用场景与边界

`RadixAttention` 在以下场景中通常收益明显：

- 大量请求共享同一 system prompt
- 固定 few-shot 示例在多个请求间重复出现
- 多轮对话系统需要反复使用相同的历史前缀

但在以下场景中，收益可能有限：

- 请求前缀差异很大，命中率低
- 请求生命周期很短，共享前缀尚未形成规模效应
- 维护索引与回收的复杂度已经接近甚至超过节省收益

因此，`RadixAttention` 更适合具有高前缀复用率的服务流量，而不是所有推理场景下的默认优化。

???+ note "RadixAttention 的 Torch 化示例"

    ```python title="radix_prefix_cache.py"
    class TrieNode:
        def __init__(self):
            self.children = {}
            self.block_ids = []
            self.ref_count = 0


    class RadixPrefixCache:
        def __init__(self):
            self.root = TrieNode()

        def match_prefix(self, token_ids):
            node = self.root
            matched_blocks = []
            for token_id in token_ids:
                if token_id not in node.children:
                    break
                node = node.children[token_id]
                matched_blocks.extend(node.block_ids)
            return matched_blocks

        def insert(self, token_ids, block_ids):
            node = self.root
            for token_id in token_ids:
                node = node.children.setdefault(token_id, TrieNode())
                node.ref_count += 1
            node.block_ids.extend(block_ids)
    ```

这个示例没有直接操作 `torch` 张量本体，而是强调另一个更重要的问题：共享 KV cache 的难点通常先出在索引结构和生命周期管理，而不是单步张量运算本身。

### KV Cache Quantization

`KV Cache Quantization` 的目标，是直接压缩缓存本身的数据体积。常见方向包括 FP8、INT8 甚至更低精度表示。

这种优化解决的核心问题有两个：

- 降低显存容量占用
- 降低 decode 阶段从显存读取历史 KV 的数据量

由于 decode 往往更偏 memory-bound，因此量化带来的收益不只是“能装下更多缓存”，还可能直接改善生成速度。

#### 容量收益与带宽收益

量化带来的收益至少有两个层面。

第一层是容量收益。更低精度意味着同样显存中可以容纳更长上下文或更多并发请求，这直接影响系统的最大服务能力。

第二层是带宽收益。decode 阶段每步都要频繁读取历史 KV，若每个元素所占字节数下降，那么每一步从显存搬运的数据量也会减少。因此，量化不只是“节省空间”，还可能直接改善 token 生成速度。

对推理系统而言，容量收益决定“能不能放下”，带宽收益决定“读得够不够快”。两者都重要，但在 decode 主导的长上下文场景里，带宽收益往往更直接体现到延迟和吞吐上。

#### 量化粒度

`KV Cache Quantization` 并不只有一种做法。按量化尺度的组织方式区分，常见思路包括：

- 整张量量化：实现简单，但尺度过粗，误差控制较弱
- 按块量化：更容易与 block-based cache 配合，在精度与实现复杂度之间折中
- 按通道或按头量化：尺度更细，精度通常更稳定，但元数据和实现复杂度更高

粒度越细，通常越容易控制误差；但同时也意味着更多 scale 元数据、更复杂的 kernel 逻辑，以及更高的实现成本。

#### 工程代价

量化的代价不只是精度损失。真实系统中，一个同样重要的问题是：量化后的 KV 如何被高效读取与使用。

若每次读取都先完整反量化成高精度张量，再执行 attention，那么很可能把带宽收益重新消耗掉。因此，工程上更理想的方式通常是：

- 让反量化尽量局部化
- 尽量把反量化与 attention kernel 融合
- 避免先恢复成大张量再单独执行后续计算

这也是为什么量化方案往往不能只看“压缩率”，还要看底层 kernel 是否能把低精度访存优势真正转化为端到端收益。

#### 精度边界

量化是否可用，还取决于误差是否会显著破坏长上下文 attention 的稳定性。KV cache 和权重量化不同，它的误差会直接作用于后续每一步 attention 的读取过程。

这意味着几个边界需要特别关注：

- 上下文越长，误差可能在更多历史读取中持续暴露
- 不同层、不同头对量化误差的敏感度可能不同
- 某些任务更依赖精细的长程依赖表示，对量化更敏感

因此，量化通常不是“压得越低越好”，而是在缓存体积、带宽收益与可接受精度之间寻找折中。

#### 适用场景

- 长上下文推理，历史 KV 占用极大
- 高并发服务，希望在同样显存下承载更多请求
- decode 已明显受限于显存带宽，希望进一步降低读取成本

若上下文较短、并发较低，或者系统瓶颈主要不在 KV 读取路径上，那么量化收益可能没有想象中明显。

它的代价则在于精度损失、反量化或低精度 kernel 的实现复杂度，以及不同模型、不同上下文长度下量化误差并不总是稳定可控。

### MQA / GQA 作为结构级优化

除了缓存管理和量化，模型结构本身也会影响 KV cache 成本。`MQA` 和 `GQA` 通过减少 KV heads 的数量，直接降低缓存体积和读取带宽需求。

这类方法的特点是，它们不是在推理引擎层面对既有缓存做压缩，而是在模型结构层面一开始就减少 K/V 的数量。因此，它们对系统的收益往往更加稳定直接，但前提是模型本身已按这种结构训练或适配。

## 调度与系统设计

KV cache 不只是单层 attention 的局部优化，它还直接影响推理系统如何组织批处理、请求迁移与资源池。

### Continuous Batching

`Continuous Batching` 的核心是 token 级调度，而不是请求级调度。系统不会等待整批请求全部完成后再统一释放资源，而是在每一轮 decode 后立即回收已经结束的请求，并把等待队列中的新请求填入空位。

这种设计的收益，是 batch 中的计算槽位和 KV cache 容量可以被更及时地复用。面对长短不一的混合请求时，`Continuous Batching` 通常能显著提高吞吐并减少空转。

它的实现难点在于调度器必须持续维护运行队列、空闲槽位和各请求对应的缓存状态。请求的加入、退出和迁移越频繁，系统越需要稳定的缓存分配与回收机制。

若从系统视角看，`Continuous Batching` 的价值不只是“把 batch 填满”，更重要的是让 KV cache 的占用和释放周期与 token 级执行节奏对齐。

### Prefill-Decode Disaggregation（PD 分离）

`Prefill-Decode Disaggregation` 指的是将 prefill 与 decode 在逻辑上或物理上拆开，分别交给不同的 GPU 或不同的节点池处理。

它的动机来自两阶段的资源需求差异：prefill 更依赖算力，decode 更依赖显存带宽。如果把两者混在同一批次和同一组设备中调度，prefill 容易挤占 decode 的资源，导致尾延迟波动；而 decode 单独运行时，又往往难以把设备算力完全利用起来。

将两者拆分后，系统可以把 compute-bound 和 memory-bound 负载分别路由到更合适的资源池中。它的代价则是 KV cache 需要在资源池之间迁移，系统架构和数据路径会更复杂，对互联带宽和调度策略也提出更高要求。

### 调度与缓存的耦合

KV cache 看起来像 attention 内部的数据结构，但在生产系统里，它同时也是调度对象。

调度器需要回答的问题通常包括：

- 这个请求当前占用了多少 KV blocks
- 哪些请求已经完成，可以回收对应缓存
- 哪些请求拥有共享前缀，可以直接复用已有 cache
- 是否值得为了隔离 prefill 与 decode 而迁移缓存

因此，KV cache 的设计不能只看单层张量布局，还必须和批处理策略、内存池管理、请求生命周期一起考虑。

!!! hint "理解 KV cache 的实现"

    写 KV cache 相关代码，关键的是写清楚数据结构和控制流：
    
    - 数据结构：KV cache 的核心是“历史 key/value 的存储结构”，无论是连续分配、分页式还是前缀共享，都是在定义一个“历史状态的组织方式”。
    - 控制流：每一步 decode 的核心流程是“生成当前 token 的 K/V，读取历史 KV cache，执行 attention，生成新 token，并把新的 K/V 追加到 cache”。这个流程的核心在于“历史 KV 的访问和更新”，而不是 attention 公式本身。

## 常见问题

### 有了 KV Cache 之后，attention 的计算是否变成了 \(O(1)\)？

不是。KV cache 只避免了历史 key/value 的重复生成，但当前 token 仍然需要读取历史位置的 KV 并完成 attention。上下文越长，当前步需要访问的历史缓存仍然越多。

因此，KV cache 减少的是重复投影成本，而不是把长上下文 attention 直接变成常数时间。

### 为什么 decode 阶段经常比 prefill 更受显存带宽限制？

prefill 阶段通常包含较大的矩阵乘法，算子更容易获得较高的数据复用和较高的算力利用率。decode 阶段每次只生成少量新 token，但仍要频繁读取较长历史对应的 KV cache。

这意味着 decode 往往不是算不动，而是“读得太多、算得不够密”。因此其瓶颈常常从 FLOPS 转移到显存带宽、缓存布局和访存效率。

### KV Cache 保存的是不是整层前向的全部中间结果？

不是。KV cache 保存的是 attention 层在历史位置上已经生成的 key 与 value，而不是整层前向中的全部激活值。

训练阶段为了反向传播，系统通常需要保留更多中间状态；推理阶段的 KV cache 则只保留对后续 attention 仍有复用价值的那部分历史表示。

### 为什么不缓存 Query？

因为 query 的作用是代表“当前这一步”去读取历史信息。当前步 attention 完成后，这个 query 通常不会在未来时间步再次被使用。

相比之下，key 和 value 会被后续多个时间步反复访问，因此它们具备跨时间步缓存的价值，而 query 没有同等收益。

### 长上下文、高并发、batch 增大分别会怎样影响 KV Cache？

长上下文会拉长单请求缓存；高并发会增加同时驻留请求数；batch 增大通常意味着同一轮需要同时维护更多请求状态。这三者都会增加显存占用，但影响路径并不完全相同。

- 长上下文主要推高单请求 cache 长度
- 高并发主要推高同时驻留请求数量
- batch 增大既可能提高吞吐，也可能让单位时间内需要管理的 KV blocks 更多

因此，系统调优时不能只盯住单一指标，而要结合上下文长度分布、请求到达模式和服务目标一起判断。

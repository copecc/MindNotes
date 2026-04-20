---
title: BPE 与分词
---

# BPE 与分词

!!! abstract

    这篇笔记整理 byte-level BPE tokenizer 的表示层、训练过程、编码解码逻辑与工程边界。正文先用小例子建立直觉，再用简化代码片段说明关键数据结构与性能优化。

## BPE 要解决什么问题

Tokenizer 不只是把文本切开。它同时决定词表规模、平均序列长度、表示完备性，以及模型需要学习的局部模式粒度。对大语言模型而言，这几个量彼此耦合：词表过大，会带来稀疏统计与 embedding 参数膨胀；词表过小，输入序列会变长，Transformer 的 attention 成本随之上升；表示若不完备，又会在长尾字符或混合语料上出现 OOV(Out-of-Vocabulary, 未登录词) 问题。

BPE 的位置可以理解为 word-level 与 pure byte-level 之间的折中。word-level 方案直接把词当作基本单位，序列较短，但面对罕见词、拼写变体和跨语言文本时不稳定。pure byte-level 方案从 256 个 byte 出发，表示完备，但序列通常过长。BPE 在 byte vocabulary 的完备性之上，通过高频 pair merge 把稳定出现的局部模式吸收到词表中，从而压缩序列长度。

在 GPT-2 或 LLaMA 风格 tokenizer 中，byte-level BPE 通常要完成三件事：训练时学习 merge 规则并构造词表；编码时把输入文本映射成 token ID 序列；解码时把 token ID 还原为 byte 串，再恢复为字符串。后面各节都围绕这三件事展开。

## byte-level 表示

Unicode 定义的是字符与 code point 的对应关系，但 tokenizer 需要的是一个稳定、有限、可统计的基本符号集合。若直接在 code point 上建词表，会同时遇到两个问题：一是字符集合过大，稀有符号多，频率统计稀疏；二是跨语言文本在 code point 层面共享的局部模式较少，不利于统一建模。

UTF-8 使表示层更适合 tokenizer。任意 Unicode 文本都可以被编码为 byte 序列，而 byte 的取值范围固定在 $0 \sim 255$。因此，byte-level tokenizer 可以从固定的 256 个基本符号出发：

$$
V_0 = \{ b \mid b \in [0, 255] \}.
$$

这样做的直接结果是，词表规模不再依赖语料中出现了多少种字符。任何输入文本都至少可以退回到 byte 序列表示，因此词表层面的 OOV 问题被消除。代价是序列更长，模型需要显式学习更多局部模式，例如空格、词干、后缀和常见标点组合。后续的 BPE merge，正是在这一表示层之上做压缩。

### byte 到可见字符的映射

很多 byte-level tokenizer 在内部使用 bytes 作为词表项，但在词表文件、merge 文件或调试输出中，原始 byte 值并不总是适合直接展示。控制字符、空白字符和不可打印 byte 会让序列化结果难以阅读，也不方便把 merge 规则保存为文本。

因此，GPT-2 一类实现通常会引入 byte-to-unicode mapping：为 256 个 byte 值构造一个一一对应的可打印字符集合，使每个 byte 都能在文本词表里稳定表示。这个映射只影响展示层，不改变训练与编码的基本对象仍然是 byte 序列。换句话说，模型学习的不是“新的字符系统”，只是 bytes 的一种文本化写法。

一个常见写法是：先把已经可打印的 byte 区间直接保留，再把其余 byte 映射到额外的 Unicode code point。这样做可以保证映射是双射，同时让词表文件中的大多数字符仍然可读。

???+ note "Byte-to-Unicode Mapping 实现"

    ```python title="bytes_to_unicode.py"
    --8<-- "learning/llm/CS336/bpe/bytes_to_unicode.py"
    ```

这个片段里，`bs` 表示优先保留为原样可见字符的 byte 集合，`cs` 表示对应的可打印字符 code point。两者按位置一一配对后，就得到 `byte -> unicode-char` 的映射。像换行符 `\n` 这样的不可打印 byte，不会以原始控制字符形式进入词表文件，而会先被转写为可见字符，再参与词表序列化。

之所以优先保留这些区间，是因为它们本身已经是稳定、可打印、常见的字符。这样可以减少词表和 merge 文件中的不可见符号，提高可读性；其余 byte 再映射到额外 code point，用来补齐双射。

### 收益与代价

byte-level tokenization 的主要收益是表示稳定。即使遇到罕见字符、混合语言文本、控制字符或者训练时未见过的符号，系统仍然总能把输入还原为 byte 序列继续处理。这一点比 word-level tokenizer 更稳定，因为后者更依赖训练时出现过的词表项。

代价同样明确。若系统只停留在单 byte 层面，序列会显著长于词或子词序列，模型需要把很多容量消耗在恢复常见局部模式上。BPE 的作用不是改变 byte-level 表示的完备性，而是在其之上显式学习高频局部结构，用更长的 token 替代重复出现的 byte 片段。

### pre-tokenization

很多 BPE 介绍把 pre-tokenization 视为训练前的准备步骤。工程上，这种说法不够准确。pre-tokenization 决定的是 merge 规则的作用边界：merge 只在单个 pre-token 内部发生，而不会跨过预分词边界。它不是附属步骤，而是 tokenizer 定义的一部分。

这样设计有两个原因。

- 不同类型的局部模式具有不同统计规律。字母串、数字串、标点串和空白序列混在一起统计，会让高频 pair 偏向无意义的跨边界组合。
- 特殊词元与结构边界必须被保护。像 `<|endoftext|>` 这样的符号承担文档边界语义，若参与普通 merge，可能被拆开或和相邻文本合并，语义就不再稳定。

常见实现会先用特殊词元的正则表达式把文本切开，再对普通片段使用 GPT-2 风格 regex 做预切分。常见 byte-level BPE 实现都遵循这条处理顺序。

???+ note "Pre-Tokenization 实现"

    ```python title="pretokenization.py"
    --8<-- "learning/llm/CS336/bpe/pretokenization.py"
    ```

这个片段包含两层边界。第一层是特殊词元的显式切分，它们作为完整 byte 串保留。第二层是普通文本内部的 regex pre-tokenization，后续 pair 统计和 merge 都只在这些局部片段内部进行。

## 训练与词表

下面用一个极小例子说明训练过程。为便于手算，这里先把基础符号写成字符级形式；真实 byte-level BPE 的底层对象是 bytes，但 merge 逻辑完全相同。

???+ example "训练过程示例"

    设预分词后的语料片段为 $\{\texttt{low},\ \texttt{lower},\ \texttt{lowest}\}$。

    初始表示为 $\texttt{low} \rightarrow (l, o, w)$，$\texttt{lower} \rightarrow (l, o, w, e, r)$，$\texttt{lowest} \rightarrow (l, o, w, e, s, t)$。

    此时相邻 pair 的频次包括 $(l, o): 3$、$(o, w): 3$、$(w, e): 2$、$(e, r): 1$、$(e, s): 1$、$(s, t): 1$。

    若第一轮选择 $(l, o)$，则得到新 token $\texttt{lo}$，语料更新为 $\texttt{low} \rightarrow (\texttt{lo}, w)$，$\texttt{lower} \rightarrow (\texttt{lo}, w, e, r)$，$\texttt{lowest} \rightarrow (\texttt{lo}, w, e, s, t)$。

    第二轮统计后，$(\texttt{lo}, w)$ 的频次为 3，因此可以继续 merge 为 $\texttt{low}$。此时表示变为 $\texttt{low} \rightarrow (\texttt{low})$，$\texttt{lower} \rightarrow (\texttt{low}, e, r)$，$\texttt{lowest} \rightarrow (\texttt{low}, e, s, t)$。

    接下来，$(e, r)$、$(e, s)$、$(s, t)$ 的频次已经不高，而 $(\texttt{low}, e)$ 的频次为 2，因此第三轮可能得到 $\texttt{lowe}$。于是 $\texttt{lower} \rightarrow (\texttt{lowe}, r)$，$\texttt{lowest} \rightarrow (\texttt{lowe}, s, t)$。

这个例子说明了两个事实。

- BPE 训练是贪心过程。每轮只根据当前频次最高的 pair 做局部决策，不优化显式的全局概率目标。
- merge 顺序必须保存。假设最终词表里同时存在 `low`、`lo`、`we` 等子词，编码阶段并不是从这些词表项里随意挑一个最长匹配，而是按训练时记录下来的 merge rank 重放历史。相同词表项集合，若 merge 顺序不同，编码结果也可能不同。

因此，训练输出通常至少包含两部分：

- `vocab: id -> bytes`，负责 token ID 与 byte 串之间的映射
- `merges: list[(bytes, bytes)]`，按创建顺序记录 merge 规则

### 特殊词元

特殊词元同时承担词表项与结构边界两种职责。若把它们与普通文本一起做 pair 统计，会出现两个问题。

- 它们会污染普通文本的频率分布，使高频 pair 反映的是结构分隔模式，而不是语言本身的局部模式。
- 它们可能被拆开，或者和前后文本组合成新的 token，使结构语义不再稳定。

工程实现通常会按同一顺序处理：训练时先把特殊词元从普通文本段中拆出，不参与普通 pair 统计；训练结束后，再把它们作为完整 byte 串追加到词表中。编码阶段也必须复用同样的边界规则。这样训练和推理看到的结构边界才是一致的。

### 训练优化

朴素 BPE 训练是一个反复执行“统计、选择、替换”的贪心循环：统计整个语料中的相邻 pair 频次；选择最高频 pair；把该 pair 的所有出现位置替换为新 token；把新 token 加入词表；继续下一轮。

???+ note "朴素训练伪代码"

    ```text
    input: corpus, vocab_size
    output: vocab, merges

    initialize vocab with all single bytes 0..255
    represent corpus as sequences of single-byte tokens

    while |vocab| < vocab_size:
        count every adjacent token pair in the whole corpus
        choose the most frequent pair (x, y)
        if no valid pair exists:
            break

        create z = x || y
        replace every adjacent occurrence of (x, y) with z
        append z to vocab
        append (x, y) to merges
    ```

若语料初始 token 总长度为 $N$，目标词表大小为 $V$，则朴素实现大致需要执行 $V - 256$ 次 merge。每轮都重扫整个语料时，总时间复杂度近似为：

$$
\mathcal{O}(N \cdot V).
$$

当语料规模达到数 GB 以上时，这个代价通常难以接受。

训练优化可以拆成三个层次来看。

- 先按频次聚合重复片段，减少统计对象的规模。
- 再维护 pair 频次、位置索引和候选堆，避免每轮线性扫描所有 pair。
- 最后只更新被当前 merge 影响的局部邻域，而不是重扫整份语料。

### 片段频次聚合

第一层优化是先做 pre-token 级别的去重与计数。与其在原始语料上直接统计 pair，不如先把重复出现的预分词片段聚合为“片段 + 频次”的形式，再在唯一片段集合内部统计相邻 pair，并用频次做加权累加。这样可以把扫描对象从原始 token 总长度压缩到唯一片段集合。

工程实现里，`token_counter` 常用于承担这一职责。它保存的不是整份语料，而是每个唯一 pre-token 对应的 token 序列及其出现次数。

### pair 计数与候选集

仅有词频聚合还不够。工程实现通常还需要同时维护三类状态。

- `pair_counter`：记录全局 pair 频次，即某个 pair 在所有片段中一共出现了多少次。
- `pair_indexes`：记录某个 pair 出现在哪些 token 序列位置，用于后续局部更新。
- `heap`：按频次快速选出当前最高频 pair，避免每轮遍历整个哈希表找最大值。

训练主循环可以压缩成下面的形式。

???+ note "BPE 训练核心循环"

    ```python title="training_core.py"
    --8<-- "learning/llm/CS336/bpe/training_core.py"
    ```

`pair_counter` 提供最新频次，`pair_indexes` 提供受影响位置，堆负责快速取候选最大项。这三个结构共同保证：每一轮可以在不重扫整个语料的前提下继续推进 merge。若缺少其中任一项，实现要么难以快速定位受影响位置，要么会在候选选择上退回到高成本扫描。

### 局部更新

堆中的旧频次会在后续 merge 后失效，因此不能简单地把堆顶直接当成当前最优 pair。实践中常见的做法是 lazy deletion：每次弹出堆顶时，先检查它记录的频次是否仍然等于 `pair_counter` 中的最新值；若不一致，说明这是旧条目，需要更新或丢弃，再继续取下一个。

更重要的优化来自局部更新。一次 merge `(A, B) -> AB` 真正影响的，不是整个语料的所有 pair，而只是被替换位置附近的邻域。若某段局部结构为

$$
\dots, X, A, B, Y, \dots
$$

则 merge 后移除的是 $(X, A)$、$(A, B)$、$(B, Y)$，新增的是 $(X, AB)$ 与 $(AB, Y)$。因此，只需要更新受影响位置附近的计数。

???+ note "局部更新逻辑"

    ```python title="apply_merge.py"
    --8<-- "learning/llm/CS336/bpe/apply_merge.py"
    ```

这就是 `pair_indexes` 存在的原因。它让实现能够快速定位“哪些 token 序列里包含当前 merge pair”，从而只修改必要的邻域，而不是重新扫描所有片段。

## 编码与解码

训练结束后，编码流程可以概括为三步：先执行与训练阶段一致的 pre-tokenization；再把每个 pre-token 写成单 byte token 序列；最后按 merge rank 逐步应用可用 merge，直到当前片段内不存在可继续合并的相邻 pair。

这意味着编码的正确性依赖两个一致性条件。

- 训练和推理必须使用同一套 pre-tokenization 规则。
- merge 规则必须按训练生成顺序重放，而不是用“最长 token 优先”或“最小词表 ID 优先”之类的替代排序。

下面的 `_apply_bpe` 过程体现了这一点。

???+ note "编码与解码实现"

    ```python title="tokenizer.py"
    --8<-- "learning/llm/CS336/bpe/tokenizer.py"
    ```

### 词表直查

若某个完整 pre-token 已经作为完整 byte 串出现在词表中，编码器可以直接返回对应 token ID，而不必再从单 byte 序列开始重放 merge。这个优化在逻辑上是安全的，因为若训练阶段确实把该片段完整合并为了单一 token，按 merge rank 重放最终也会收敛到同样结果。

### merge rank 与最长匹配的差异

BPE 的决策依据是 merge rank，也就是训练阶段记录下来的 pair 优先级。WordPiece 的常见实现更接近局部最长匹配：在当前位置选择词表中最长可匹配的子串。二者都能产生子词分段，但规则不同，因此相同词表下也可能得到不同编码结果。

### 片段级缓存

自然语言语料满足明显的长尾分布，高频片段会重复出现。于是编码器通常维护 `cache[pretoken] = token_ids`。命中缓存时，可以直接复用已有编码结果，避免重复执行 merge 重放。这个优化不改变语义，只减少重复片段的重复计算。

### 解码

编码阶段之所以复杂，是因为它需要把原始文本映射到由 merge 历史定义的 token 空间中。解码阶段则简单得多：把 token ID 映射回 byte 串，按顺序拼接，再做一次 UTF-8 decode：

$$
\mathrm{text} = \mathrm{decode}_{utf8}(b_1 \Vert b_2 \Vert \dots \Vert b_n).
$$

唯一需要额外考虑的是，用户提供的 token ID 序列未必对应合法 UTF-8。某些 byte 组合可能形成非法 continuation pattern。工程实现通常使用 `errors='replace'`，把非法序列恢复为 replacement character，而不是让解码过程直接失败。

## 长文件编码

短文本示例很容易掩盖 tokenizer 在大语料上的真实成本。若把数十 GB 文本一次性读入内存再整体编码，内存占用会很高，也会让上游流水线失去流式处理能力。因此，实现通常需要区分短文本接口与长文件接口。

- `encode(text) -> list[int]` 适合交互式短文本。
- `encode_iterable(iterable) -> Iterator[int]` 或分块接口更适合大规模语料预处理。

文件编码接口通常会按特殊词元边界分块、并行编码、最后再聚合结果。这里的关键不是“把文件切开”本身，而是保证切块不会改变 tokenizer 的语义边界。

???+ note "文件编码实现"

    ```python title="file_encoding.py"
    --8<-- "learning/llm/CS336/bpe/file_encoding.py"
    ```

这里有三个工程点需要单独强调。

### 分块边界

文件切块不能只按固定字节位置硬切。若边界恰好落在 `<|endoftext|>` 中间，或者截断了需要完整保留的结构标记，不同块之间的 pre-tokenization 结果就会变化。`find_chunk_boundaries` 的作用正是沿着特殊词元搜索可安全切分的位置。

### `numpy.memmap`

`numpy.memmap` 适合处理同构数值数组，例如最终写出的 token ID 序列。但它并不自动解决文本预处理问题。BPE 编码的主要计算仍然发生在 regex pre-tokenization、哈希查找与 merge 重放阶段。如果为了做文本处理而频繁在 memmap、NumPy 数组、Python `bytes` 和 `str` 之间转换，额外的数据结构转换开销可能抵消 memory mapping 带来的收益。

### I/O 与 CPU 解耦

大规模语料编码的瓶颈通常不在磁盘读取，而在 CPU 侧的预切分、哈希查找和 merge 重放。更有效的做法是把 I/O 与 CPU 计算解耦：主进程负责分块读取并提交任务，工作进程池负责执行 tokenization。这样做往往比单纯追求更复杂的文件映射方式更有效。

## 局限与对比

BPE 是确定性的贪心算法。每一轮训练都根据当前 pair frequency 选择一个局部最优 merge，并把这次决策固定进后续词表与 merge 顺序中。它优化的是局部统计模式，而不是显式的全局概率目标。

这种设计的优点是实现直接、训练稳定、编码结果唯一。代价在于早期 merge 决策会长期影响后续词表结构。某些频率高但语义价值有限的局部模式可能较早进入词表，而一些频率较低、但在下游任务中更有解释力的结构则可能被更晚处理，甚至始终保留为更细粒度的组合。

与之相比：

- WordPiece 常见实现更接近基于词表的局部最长匹配，编码规则不等于重放一串 merge 历史。
- Unigram 通常基于显式概率目标建模，允许同一片段存在多个候选切分，并通过目标函数比较不同分段。

这几种 tokenizer 都在做子词分段，但其训练目标、编码规则与工程约束并不相同。理解 BPE 时，最关键的是记住：它的核心对象不是“最终的一组子词”，而是“一组带顺序的 merge 决策”。

???+ tip "实现提示"

    检查 BPE 实现时，通常至少要确认以下几点。

    - 训练与编码是否共享同一 pre-tokenization 规则。
    - 特殊词元是否始终被隔离，不参与普通 merge。
    - merge rank 是否被保留，并在编码阶段按原顺序重放。
    - 编码是否允许词表直查，但不把它误写成一般性的最长匹配。
    - 解码是否按 bytes 拼接再做 UTF-8 恢复，而不是直接拼接字符串片段。
    - 长文件编码是否正确处理分块边界，而不是在任意字节位置切开。

    若这些条件不满足，即使词表内容看起来相近，训练与推理的实际分段结果也可能已经偏离原始算法定义。

???+ note "实现链接"

    见 [CS336 assignment1-basics](https://github.com/copecc/CS336/tree/main/assignment1-basics){target=_blank}。

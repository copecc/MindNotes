---
title: Serving
---

# Serving

!!! abstract

    本文整理 language model 推理服务中的请求进入、准入控制、batching、prefill 与 decode 调度、cache 生命周期、资源隔离，以及这些机制如何共同决定吞吐、延迟与稳定性。这里讨论的不是单次前向如何计算，而是模型如何在真实在线系统中被稳定、高效地提供服务。

## Serving 的目标

推理服务的目标是在有限 GPU 资源下，同时兼顾单请求延迟、系统整体吞吐、最大并发请求数、显存占用与尾延迟稳定性。

因此，serving 的问题天然是系统级折中，而不是单一算子优化。即使底层模型和 attention kernel 已经足够快，若请求进入、准入、调度、缓存和资源隔离设计不合理，整体服务质量仍然会明显下降。

## 请求进入系统之后

从系统视角看，一个推理请求通常会经历以下链路：进入前端队列、通过准入控制、进入 prefill、转入 decode、持续占用 KV cache、命中停止条件后释放相应资源。

若把这条链路展开，可以得到更细的阶段划分：

1. 请求到达前端入口并进入等待队列
2. 系统根据当前负载决定是否立刻接纳、排队、降级或拒绝
3. 被接纳的请求进入 prefill，处理完整 prompt 并建立初始 KV cache
4. 请求转入 decode，逐 token 生成输出
5. 请求结束后释放 decode 槽位与相关缓存状态

serving 的很多问题，并不出在某一步单独过慢，而是出在这条链路的各个阶段之间没有形成稳定配合。例如，前端队列过长会推高 TTFT；decode 槽位长期被长请求占用会拖慢新请求进入；cache 回收不及时又会进一步压缩准入能力。

## 核心指标

推理服务最常见的几个指标分别刻画不同阶段的系统行为。

### TTFT

TTFT，Time To First Token，表示用户发起请求到看到第一个 token 的时间。它主要受以下因素影响：

- 队列等待时间
- 准入控制是否延迟请求进入 prefill
- prefill 的计算时间
- 请求是否需要等待凑批
- 长 prompt 是否抢占了当前资源

因此，TTFT 更接近“请求从进入系统到开始得到响应”的综合结果，而不只是 prefill 本身的耗时。

### TPOT

TPOT，Time Per Output Token，表示 decode 阶段平均每生成一个 token 所需的时间。它更能反映 decode 的稳定性与带宽利用率。

TPOT 主要受以下因素影响：

- KV cache 读取成本
- decode batch 的大小与稳定性
- 显存带宽是否成为瓶颈
- 调度器是否能稳定复用 decode 槽位
- 是否使用量化、GQA、MQA、投机解码等优化

### Throughput

throughput 指单位时间内系统能完成多少 token 生成或多少请求处理。它更偏向系统整体资源利用率，而不是单个请求体验。

在离线批处理或高并发 API 场景下，throughput 往往是核心指标；在交互式助手场景下，通常不能只追求 throughput，而忽略 TTFT 与尾延迟。

### 尾延迟

平均延迟往往无法反映最差体验。真实系统更关注 P95、P99 一类尾延迟指标，因为系统中最慢的那部分请求通常来自：

- 超长 prompt
- 异常大的 decode batch
- 某些请求占用了过多 cache
- 调度器长期偏向吞吐而压缩交互请求优先级
- 某些缓存未能及时回收，导致后续请求长期等待

因此，serving 优化不仅要看平均值，还要看延迟分布是否稳定。

## 准入控制

当请求到达速度超过处理能力时，系统不能只回答“是否还有队列空间”，还必须回答“是否值得让这个请求现在进入系统”。从 serving 视角看，这类机制通常统一归入 admission control。

系统在高负载时通常不会把所有请求一视同仁，而是结合队列长度、请求长度、业务优先级、当前 decode 槽位和显存水位决定是否接纳请求。

### 准入动作

一个请求到达后，系统通常有四种基本动作：

- 立即接纳，直接进入当前执行链路
- 暂时排队，等待后续资源释放
- 降级处理，例如限制最大输出长度或改走低优先级池
- 直接拒绝，避免系统继续恶化

这意味着 admission control 不是单纯的过载保护，而是系统维持 TTFT、TPOT 和公平性目标的前置决策层。

### 准入依据

仅看队列长度通常是不够的，因为 serving 的真实瓶颈往往出现在后续阶段，而不是入口本身。系统至少需要关注：

- prefill 是否还有可接受的等待空间
- decode 槽位是否接近饱和
- 显存中还能否容纳更多活跃请求的 KV cache
- 当前是否有长 prompt 或长输出请求正在放大尾延迟

因此，准入控制天然和后续调度耦合。一个表面上“只是排队”的请求，实际上可能对应未来数十秒的 decode 占用与持续 cache 驻留。

### 长请求限制

长 prompt 请求往往在进入 prefill 之前就需要特殊对待。因为它们不仅会带来更高的首 token 计算成本，也会在进入 decode 后占据更长时间的 cache 空间。

因此，很多系统会对超长 prompt 单独限流、分层或降级。这并不是因为系统无法处理长请求，而是因为在混合负载下，长请求过多会显著恶化整体交互体验。

???+ note "admission control 示例"

    ```python title="admission_control.py"
    class AdmissionController:
        def __init__(self, max_waiting, max_decode_slots, max_kv_blocks):
            self.max_waiting = max_waiting
            self.max_decode_slots = max_decode_slots
            self.max_kv_blocks = max_kv_blocks

        def decide(self, request, scheduler):
            if len(scheduler.waiting_prefill) >= self.max_waiting:
                return "reject"

            if request.prompt_len > scheduler.long_prompt_limit:
                return "downgrade"

            if scheduler.decode_slots_in_use() >= self.max_decode_slots:
                return "queue"

            if scheduler.kv_blocks_in_use() + request.estimated_kv_blocks > self.max_kv_blocks:
                return "queue"

            return "admit"
    ```

这个示例强调两个判断：请求是否值得现在进入系统，以及它进入后是否会立即挤压 decode 槽位与 KV cache 余量。真实系统通常还会继续叠加业务优先级、租户预算和预测输出长度等信息。

## Batching

batching 的做法，是把多个请求打包进同一批前向计算中，以提高硬件利用率。对 GPU 而言，这通常能减少空转，提高总吞吐。

但 batch 变大并不意味着一切都更好，因为系统需要为凑批付出等待成本。请求长度差异较大时，padding、批内不均衡和资源占用不对称也会增加。

### 静态批处理

静态批处理要求一整批请求同时进入、同时推进。它的实现最简单，也容易和常规训练式前向兼容。

但这种方式的问题也很明显：

- 请求必须等待系统凑够 batch
- batch 中最短请求必须等待最长请求
- 不同长度请求之间容易形成 padding 浪费

因此，静态批处理更适合离线场景，不适合高并发、低延迟的交互式服务。

### 连续批处理

连续批处理，又称 continuous batching 或 in-flight batching，允许系统在已有 batch 运行过程中动态加入新请求，并在旧请求结束后立即回收位置。

它的核心收益是：batch 的生命周期不再绑定于最长请求，而是变成 token 级动态调整。这样可以显著减少批内空洞和 padding 浪费。

从执行过程看，连续批处理至少包含两个关键动作：

- 当某些请求结束后，系统立即释放对应槽位
- 当有空闲槽位出现时，调度器尽快把等待中的合适请求送入下一轮执行

这意味着“谁什么时候进入 batch、谁什么时候退出 batch”不再是批次边界上的一次性决定，而是每一轮 decode 之后都可能发生变化。

因此，连续批处理的价值不只是提升平均吞吐，更重要的是提高槽位复用速度，并减少长短请求混合时的资源空洞。它和 KV cache 管理天然耦合，因为请求能否灵活进出 batch，取决于系统能否同步完成 cache 分配、状态迁移与回收。

## Prefill 与 Decode 调度

从 serving 角度看，prefill 与 decode 不只是模型内部两个阶段，也是两类不同的服务负载。

### Prefill 负载

prefill 需要处理完整 prompt。若 prompt 很长，它通常会带来：

- 较高的单次算力消耗
- 更长的首 token 等待
- 更明显的批内长度不均衡

因此，长 prompt 请求经常成为 TTFT 抖动的重要来源。

### Decode 负载

decode 是逐 token 的持续生成过程。单步计算看似较轻，但它需要高频调度、持续读取权重与 KV cache，因此非常容易暴露显存带宽和调度器开销问题。

在真实系统中，decode 往往不是算不动，而是带宽、调度和 cache 管理难以始终保持高效。

### 阶段差异

如果把 prefill 和 decode 完全混合调度，prefill 的高算力需求会干扰 decode 的稳定节奏，decode 的高频小步调度也会让 prefill 更难形成高效大 batch。

因此，prefill 与 decode 的差异不只是模型内部的计算阶段不同，更意味着两者对系统提出了不同资源需求：

- prefill 更强调一次性算力利用
- decode 更强调稳定的带宽与调度节奏
- 二者对 batch 组织和资源分配的理想形态并不一致

### 调度策略

推理服务的核心并不只是“批处理”，而是如何决定谁先执行、谁延后执行、哪个请求占用多少资源。

#### FIFO

最简单的方式是 FIFO，即按到达顺序处理请求。它实现简单、公平性强，但在负载复杂时会出现明显问题：一个超长 prompt 或超长生成请求可能拖慢后续大量短请求。

#### 长短请求分层

一些系统会把请求按 prompt 长度、预估输出长度或业务优先级分层。这样做的目标，是避免长请求完全主导调度器。

典型收益包括：

- 降低短请求的 TTFT
- 避免超长请求拖垮交互式服务体验
- 提高同类请求凑批的相似度

#### 优先级调度

若系统同时服务不同业务，例如在线聊天、批量生成和内部工具请求，通常还需要优先级调度。高优先级请求可能被优先送入 prefill 或 decode，而低优先级请求则在系统空闲时批量处理。

这类机制的重点不是绝对公平，而是在业务目标上把资源分配给更重要的流量。

## Cache 生命周期

在 serving 系统中，cache 不是“分配完就结束”的静态资源，而是伴随请求生命周期不断经历创建、共享、追加、回收、驱逐与重路由的动态状态。

因此，系统必须管理的不只是 cache 能否放下，还包括：

- 何时回收空闲 blocks
- 何时主动驱逐低价值缓存
- 新请求是否可以路由到已有前缀
- 不同 worker 上的缓存是否仍然值得保留

### Block Recycling

当请求结束，或者某些尾部状态已经不再需要时，系统必须把对应 blocks 返回到 free pool。若回收速度不够快，即使总显存看起来仍然充足，调度器也可能因为拿不到可复用块而无法及时接纳新请求。

这意味着 block recycling 不是简单的释放动作，而是直接影响准入能力和槽位复用速度的 serving 机制。回收过粗会让空闲资源滞后暴露，回收过慢则会让 TTFT 和排队时间被动上升。

工程上通常需要持续处理几个问题：

- 请求何时被判定为真正完成
- 引用关系何时可以安全释放
- 部分复用后是否会形成碎片或零散空洞

### Cache Eviction

当显存压力升高时，系统未必能一直等到缓存自然失效或请求自然结束。此时，就可能需要主动 reclaim 某些缓存状态，这类机制通常可以归入 cache eviction。

被驱逐的对象通常包括：

- 长时间不活跃的会话缓存
- 低优先级请求占用的缓存
- 命中价值较低但仍驻留的共享前缀缓存

eviction 的本质，是用未来可能的复用收益换取当前的准入能力和系统稳定性。因此，它本身就是一种系统级权衡，而不是单纯的内存清理。

需要区分的是：

- recycling 是请求自然结束后的正常回收
- eviction 是系统在压力下提前回收仍可能被再次使用的状态

这类策略会间接影响 TTFT 和尾延迟。因为一旦驱逐过于激进，后续相似请求就失去复用路径，需要重新 prefill；但如果驱逐过于保守，又会挤压新请求进入系统的空间。

### Prefix Cache Routing

当系统支持前缀共享后，一个新请求进入系统时，调度器不仅要决定“是否接纳”，还要决定“应不应该把它路由到一个已有的共享前缀上”。这就是 prefix cache routing。

它不是单纯的 cache 命中判断，而是一种 serving 路由决策，因为它会改变请求接下来经过的 worker、prefill 负载以及后续 decode 队列的形态。

prefix routing 通常需要知道：

- 当前有哪些前缀仍然驻留
- 哪个 worker 或资源池持有这些前缀
- 这些前缀是否仍然足够热门，值得继续复用

因此，prefix reuse 改变的不只是内存占用，还会改变请求路由和调度器的选择空间。若系统把一个请求路由到已有共享前缀上，它可以减少重复 prefill；若命中成本过高、前缀过冷或路由代价过大，系统也可能选择直接重新计算。

## 资源隔离

随着系统规模变大，单一 GPU 池往往难以同时满足低延迟交互和高吞吐离线任务。因此，资源隔离会成为 serving 架构的重要部分。

### PD 分离

一种典型做法是将 prefill 与 decode 分离到不同资源池。prefill 池更强调算力利用，decode 池更强调显存带宽和稳定调度。

这样做的收益包括：

- 降低长 prompt 对 decode 的干扰
- 让 decode 的 TPOT 更稳定
- 让 prefill 更容易形成高效大 batch

### 多租户隔离

若系统服务多个业务方，还可能进一步做多租户隔离，例如给不同业务分配独立 GPU 池、独立队列或独立优先级预算。这样可以避免一个业务的流量高峰拖垮整个推理平台。

资源隔离和 cache 生命周期并不是独立问题。一个资源池是否能稳定服务某类流量，不仅取决于算力和带宽，也取决于该池内 cache 是否能够及时回收、是否容易被长会话长期占满，以及前缀共享是否被路由到合适位置。

## 与 KV Cache 的关系

serving 和 KV cache 不是两个独立主题。实际上，serving 的很多调度与吞吐问题，本质上都由 KV cache 约束。

### 并发上限

一个请求一旦进入 decode，就会持续占用其对应的 KV cache 空间。并发用户数并不只由模型权重是否放得下决定，还取决于显存里还能容纳多少活跃请求的缓存。

### 调度自由度

连续批处理之所以能成立，是因为系统能够在请求级别管理和回收 KV cache。若缓存分配方式僵硬、回收成本高、碎片严重，调度器的灵活性也会被显著限制。

### 尾延迟来源

一些尾延迟并不是纯粹由计算慢造成，而是因为：

- 某些请求占用过大的 cache
- cache 碎片导致无法快速接纳新请求
- 长上下文请求使 decode batch 不稳定

因此，serving 层的稳定性在很大程度上依赖于底层 KV cache 的组织方式，但二者关注的问题并不相同：`KVCache.md` 更关注缓存如何组织与复用，`Serving.md` 更关注这些缓存约束如何反过来塑造调度、准入和系统行为。

## 常见瓶颈

推理服务中的主要瓶颈通常来自以下几类。

### KV cache 占用过高

若 cache 占用过高，并发请求数就会被直接限制。系统看起来可能还有算力，但新请求无法进入，因为显存已经被活跃会话的历史上下文占满。

### 请求长度差异过大

当 prompt 长度或输出长度差异过大时，batch 内部会变得高度不均衡，调度器更难维持稳定吞吐。短请求容易被长请求拖住，长请求又可能长期占用 decode 槽位。

### 首 token 延迟过高

TTFT 过高通常来自排队与 prefill 叠加，而不是单纯模型算得慢。尤其是长 prompt、多租户混跑和静态凑批场景下，首 token 延迟往往是最早暴露问题的指标。

### Decode 阶段利用率不稳定

decode 每轮只生成少量 token，调度频繁，且对显存带宽高度敏感。若 batch 管理不稳定、KV cache 读取路径不高效或请求进出过于频繁，GPU 很难始终保持高效运行。

### 网络与跨节点传输

在 PD 分离、多机推理或张量并行部署中，网络传输也会成为瓶颈。例如：

- KV cache 从 prefill 节点传输到 decode 节点
- 多个 GPU 之间做张量并行同步
- 前端调度器与后端 worker 的状态同步

因此，serving 不只是单机 GPU 调优，还需要考虑系统级数据路径。

## 常见优化方向

现代推理服务通常会同时使用多种优化手段，而不是只依赖单一策略。

### 批处理优化

- 使用连续批处理替代静态批处理
- 尽量让同类长度请求凑批
- 按 TTFT 或优先级控制 prefill 队列

### 缓存优化

- 使用分页 KV cache 降低碎片
- 使用前缀共享与 prefix cache routing 减少重复 prefill
- 通过 block recycling 提高空闲缓存回收速度
- 在高压下通过 eviction 策略换取准入能力
- 使用 KV cache 量化降低显存和带宽压力

### 计算优化

- 使用模型量化降低权重读取成本
- 使用投机解码减少 decode 串行步数
- 使用 GQA/MQA 降低 decode 阶段 KV 读取规模

### 架构优化

- 对 prefill 与 decode 做资源分离
- 在多业务场景中加入优先级和租户隔离
- 在高负载时启用背压与降级策略

这些优化的共同目标，不是让某一次请求跑得最快，而是让系统在真实混合负载下仍能保持稳定吞吐和可接受延迟。

## 简化调度示例

下面的代码不是生产级 serving 框架，而是为了说明一个最小调度器如何同时处理 waiting queue、running batch 与请求完成后的槽位回收。

???+ note "连续批处理调度示例"

    ```python title="serving_scheduler.py"
    from collections import deque


    class Request:
        def __init__(self, req_id, prompt_tokens, max_new_tokens):
            self.req_id = req_id
            self.prompt_tokens = prompt_tokens
            self.output_tokens = []
            self.max_new_tokens = max_new_tokens
            self.finished = False

        def append_token(self, token_id, eos_token_id):
            self.output_tokens.append(token_id)
            if token_id == eos_token_id or len(self.output_tokens) >= self.max_new_tokens:
                self.finished = True


    class ServingScheduler:
        def __init__(self, max_batch_size):
            self.max_batch_size = max_batch_size
            self.waiting_prefill = deque()
            self.running_decode = []

        def submit(self, request):
            self.waiting_prefill.append(request)

        def run_prefill(self, model):
            admitted = []
            while len(admitted) < self.max_batch_size and self.waiting_prefill:
                admitted.append(self.waiting_prefill.popleft())

            if not admitted:
                return

            model.prefill(admitted)
            self.running_decode.extend(admitted)

        def run_decode_step(self, model, eos_token_id):
            if not self.running_decode:
                return

            next_tokens = model.decode(self.running_decode)
            next_running = []

            for req, token_id in zip(self.running_decode, next_tokens):
                req.append_token(token_id, eos_token_id)
                if not req.finished:
                    next_running.append(req)

            self.running_decode = next_running
    ```

这个示例刻画了三个最基本的系统动作：

- 新请求先进入等待队列
- prefill 结束后请求进入 decode 集合
- decode 每轮回收已完成请求，并释放出新的 batch 槽位

???+ note "cache 生命周期钩子示例"

    ```python title="cache_aware_scheduler.py"
    class CacheAwareScheduler(ServingScheduler):
        def __init__(self, max_batch_size, block_pool, prefix_router):
            super().__init__(max_batch_size)
            self.block_pool = block_pool
            self.prefix_router = prefix_router

        def submit(self, request):
            route = self.prefix_router.lookup(request.prompt_tokens)
            if route is not None:
                request.attach_shared_prefix(route.worker_id, route.block_ids)
            self.waiting_prefill.append(request)

        def recycle_finished(self):
            still_running = []
            for req in self.running_decode:
                if req.finished:
                    self.block_pool.release(req.private_block_ids)
                    self.prefix_router.dec_ref(req.shared_prefix_id)
                else:
                    still_running.append(req)
            self.running_decode = still_running

        def maybe_evict(self):
            if not self.block_pool.near_capacity():
                return

            victim = self.prefix_router.pick_evictable_prefix()
            if victim is not None:
                self.block_pool.release(victim.block_ids)
                self.prefix_router.remove(victim.prefix_id)
    ```

这个示例额外补上了三类控制流：新请求进入时先做 prefix routing，请求结束时回收私有 block 与共享前缀引用，显存逼近上限时主动驱逐低价值前缀。这样才能把 admission、continuous batching 与 cache lifecycle 真正连接成一条完整链路。

真实推理引擎会在此基础上继续加入优先级调度、KV cache 分配、PD 分离、超时控制、限流和多机路由等机制。

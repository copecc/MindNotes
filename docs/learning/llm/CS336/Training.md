---
title: 训练
---

# 训练

!!! abstract

    本文整理 language model 的训练目标、训练循环与训练工程问题，聚焦 cross-entropy、perplexity、学习率调度、梯度稳定性、数据加载、checkpoint 与实验记录。这里讨论的是训练如何稳定、可恢复、可观测地运行，而不是 optimizer 内部如何更新参数。

## 训练目标

对 decoder-only language model 而言，训练阶段的基本问题不是“参数怎么更新”，而是如何把 token 序列组织成监督信号，并在稳定、可恢复、可观测的训练循环中不断降低目标函数。

若序列为

$$
(x_1, x_2, \dots, x_T),
$$

则语言模型训练通常等价于让每个位置根据前文预测下一个 token。也就是说，训练循环真正反复执行的是：取 batch、前向计算 logits、计算 loss、反向传播、更新参数、记录指标。

## Cross-Entropy

训练时，模型会在每个位置输出对整个词表的 logits，再与目标 token 计算 cross-entropy。若某个位置的目标 token 为 $y$，对应 logits 为 $z$，则负对数似然可以写成

$$
-\log p(y \mid x) = -z_y + \log \sum_j e^{z_j}.
$$

这也是为什么实现里经常讨论 log-sum-exp 的数值稳定性。cross-entropy 直接刻画了模型在目标 token 上分配了多少概率，因此它是训练目标，而不是仅用于展示的评估指标。

## Perplexity

perplexity 是平均负对数似然的指数形式。若平均 loss 为 $\ell$，则

$$
\mathrm{PPL} = e^{\ell}.
$$

它不是新的优化目标，而是对同一训练信号的另一种刻画方式。好处是更接近“平均有效候选数”的量纲，但在分析训练曲线时仍要回到 loss 本身理解变化来源。

## 反向传播与梯度

前向阶段得到 loss 后，反向传播会沿计算图把梯度传回各层参数。这里需要区分两个层次：

- loss 定义了训练目标
- backward 给出了目标对参数的梯度

训练系统关心的，不只是梯度是否能算出来，还包括梯度是否稳定、是否需要累积、是否会出现异常峰值，以及这些问题会怎样影响后续更新。

## Gradient Accumulation

当单卡或单次前向无法容纳足够大的 batch 时，常见做法是把多个 micro-batch 的梯度累积起来，再统一执行一次 optimizer step。这样做的目的，是在不提高单次显存占用的前提下，近似更大的有效 batch size。

它改变的是一次参数更新对应多少样本，而不是模型定义本身。因此，分析训练行为时，需要同时区分：

- micro-batch size
- gradient accumulation steps
- effective batch size

## Learning Rate Schedule

学习率调度控制的是不同训练阶段允许多大的更新步长。它和 optimizer 一起决定单步更新的实际尺度，但逻辑上属于训练流程的一部分，因为它依赖训练步数与阶段划分。

### Warmup

warmup 的作用，是缓解训练初期统计量尚未稳定时的大步长震荡。尤其在 Adam 类方法、较大 batch 或较深模型下，直接使用目标学习率往往会让最早阶段过于剧烈。

### Cosine Decay

cosine decay 常用于后期逐步降低学习率，使参数在接近收敛时减少抖动。它的意义不在于某个特殊三角函数本身，而在于给训练后半段提供平滑、连续的降速过程。

## Gradient Clipping

gradient clipping 的目的不是改变优化目标，而是限制异常大梯度对单步更新的破坏。当某一步梯度范数远高于正常范围时，若直接更新，模型参数可能被一次性推离稳定区间。

因此，clipping 应与学习率一起理解：

- 学习率决定正常更新步长
- clipping 决定异常更新最多能大到什么程度

## Batch Construction

训练语言模型时，原始文本通常会先被 tokenizer 编码成长 token 序列，再切成固定长度窗口组成 batch。这里的关键不是“句子边界是否完整”，而是模型是否能在固定上下文窗口内学习局部与跨位置统计关系。

很多实现会把多篇文档拼接成连续 token 流，再从中采样长度为 $T$ 的片段。这样做的代价，是少量跨文档边界的上下文并不自然；收益则是数据管线更简单、吞吐更高。

## Data Loading

数据加载的目标不是改变训练定义，而是持续、高效地向训练循环提供 batch。对大语料而言，数据管线需要同时考虑：

- 存储格式
- 随机采样方式
- CPU 到 GPU 的搬运成本
- 是否能避免一次性把全部语料读入内存

例如 `np.memmap` 常用于把大规模 token 数据映射到磁盘文件，从而避免完整载入内存。这类问题虽然不改变模型数学定义，却直接影响训练吞吐。

## Checkpoint 与恢复训练

可恢复训练通常不只保存模型参数，还要保存：

- optimizer state
- 当前步数
- 学习率调度器进度
- 随机数状态

若缺少其中一部分，恢复后的训练轨迹可能与中断前不一致。也就是说，checkpoint 的语义不是“能继续跑”，而是“尽可能继续同一条训练轨迹”。

## Training Loop

训练循环可以抽象为以下顺序：

1. 取一个 batch
2. 前向计算 logits 与 loss
3. backward 计算梯度
4. 必要时做 gradient accumulation 或 clipping
5. optimizer step
6. scheduler step
7. 记录日志、定期验证、定期保存 checkpoint

这条主线把 loss、gradient、optimizer、schedule、checkpoint 串成了一个闭环，因此训练文档适合按这条顺序组织。

## Validation 与实验记录

训练阶段不能只看某一次生成样例。更稳定的做法是同时记录训练损失、验证损失、训练步数与 wallclock，并在固定频率上做验证。

做 ablation 时，应尽量满足两个条件：

- 一次只改一个关键变量
- 保持数据、训练步数与评估口径一致

否则，即使指标变化明显，也很难把差异归因到具体设计选择。

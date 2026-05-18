---
title: Softmax
tags:
  - CUDA
  - Kernel
  - Softmax
---

# Softmax

## 类型定位

Softmax 是将连续实数向量映射成概率分布的最基础算子，在深度学习特别是在 Transformer 的 Attention 计算、多分类输出等核心环节出镜率极高。从 CUDA 算子分类上看，Softmax 是结合了局部 Reduction（寻找分布最大值防止溢出、求指数和）与后续的 Elementwise（整体放缩）的综合操作。

## 优化方法

- **两阶段或三阶段 Safe Softmax**：利用减去行最大值（`max`）再计算 `exp` 避免数值上溢。传统的实现会要求三次全局存储访问（分别求局部 Max、求指数及和、乘法还原覆盖输出）。
- **合并共享内存或寄存器（Online Softmax）**：利用一次循环内同时追踪维护新引入的 `max` 和放缩后的修正 `sum`，这大幅降低了显存读写次数，并成为 FlashAttention 内部最重要的组成机制之一。

## 题目索引

- `softmax_basic`
- `online_softmax`

## softmax_basic

传统的 “Safe Softmax”，对数据执行两次特征扫描。依靠 Block / Warp 层次的规约算子分别求出极大值和缩放和，然后再写回全局显存。

$$
y_i = \frac{e^{x_i - \max(x)}}{\sum_{j} e^{x_j - \max(x)}}
$$

???+ note "softmax_basic 实现"

    === "Basic"

        ```cpp title="softmax_basic"
        --8<-- "learning/cuda/kernel/softmax_basic.cu"
        ```

## online_softmax

Online Softmax 修改了标准版 Safe Softmax 必须多轮遍历内存的限制。它通过在单次前向遍历中动态修正以过去的局部最大值为基准的指数和，从而实现规约阶段的合并。

**数学推导：**

给定输入向量 $X = [x_1, x_2, \dots, x_N]$。常规的 Safe Softmax 需要先求得全局最大值 $m_N = \max_{j=1}^N x_j$，然后计算归一化分母 $l_N = \sum_{j=1}^N e^{x_j - m_N}$。
假设我们在遍历到第 $i$ 个元素时，已知的局部最大值为 $m_i = \max(m_{i-1}, x_i)$，此时的局部指数和为 $l_i = \sum_{j=1}^i e^{x_j - m_i}$。

当处理下一个状态时，根据指数的代数性质，我们可以将旧的局部和 $l_{i-1}$ 映射到新的更大基准 $m_i$ 上：

$$
\begin{aligned}
l_i &= \sum_{j=1}^i e^{x_j - m_i} \\
&= e^{x_i - m_i} + \sum_{j=1}^{i-1} e^{x_j - m_i} \\
&= e^{x_i - m_i} + \sum_{j=1}^{i-1} e^{x_j - m_{i-1} + m_{i-1} - m_i} \\
&= e^{x_i - m_i} + e^{m_{i-1} - m_i} \sum_{j=1}^{i-1} e^{x_j - m_{i-1}} \\
&= e^{x_i - m_i} + l_{i-1} e^{m_{i-1} - m_i}
\end{aligned}
$$

通过此递推公式，随着每次遇到新的更大值更新 $m_{new}$，原先积累的部分和 $l_{old}$ 会按比例自然衰减。最终只需要访问全局数据两次，甚至可以彻底结合前序矩阵乘操作变成完全熔合的一趟（一边算分布一边加权输出）。

???+ note "online_softmax 实现"

    === "Basic"

        ```cpp title="online_softmax"
        --8<-- "learning/cuda/kernel/online_softmax.cu"
        ```

## 边界与局限

若单行的数据量超过了可并行管理的 Block 共享内存或寄存器限度，传统的 Warp 线程级规约可能需要引入多趟 Kernel 以及额外的 Global Memory 同步。

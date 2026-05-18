---
title: Elementwise
tags:
  - CUDA
  - Kernel
  - Elementwise
---

# Elementwise

## 类型定位

Elementwise kernel 的输出位置相互独立，每个输出元素只依赖同一位置或同一逻辑位置上的输入元素。它通常采用 `1D grid + 1D block`，让一个线程负责一个元素。

这类 kernel 的重点不是同步，而是把输入到输出的变换写清楚：线程读取 $x_i$、或读取 $(a_i, b_i)$，计算 $f(x_i)$ 或 $f(a_i, b_i)$，再写回输出位置 $y_i$ 或 $c_i$。

## 优化方法

- 使用连续线程访问连续地址，保持 coalesced global memory access。
- 对大输入使用 grid-stride loop，让同一个 launch 结构覆盖更长数组。
- 当输入和输出满足 16 字节对齐时，使用 `float4` 一次处理四个标量。
- 对多个简单 elementwise 操作可考虑 fusion，减少中间写回和额外 launch。

## 题目索引

- `vector_addition`
- `relu`
- `leaky_relu`
- `sigmoid`
- `silu`
- `swiglu`
- `value_clipping`
- `color_inversion`

## vector_addition

`vector_addition` 接收两个等长向量，输出逐元素加法结果：

$$
C_i = A_i + B_i
$$

线程索引 `idx` 直接对应向量下标，因此它是最基础的二输入 elementwise 结构。

???+ note "vector_addition 实现"

    === "Basic"

        ```cpp title="vector_addition_basic"
        --8<-- "learning/cuda/kernel/vector_addition_basic.cu"
        ```

    === "Advanced"

        ```cpp title="vector_addition_advanced"
        --8<-- "learning/cuda/kernel/vector_addition_advanced.cu"
        ```

## relu

`relu` 对每个输入元素执行截断变换：

$$
y_i = \max(x_i, 0)
$$

每个线程只读取一个输入元素并写回同一位置的输出。

???+ note "relu 实现"

    === "Basic"

        ```cpp title="relu_basic"
        --8<-- "learning/cuda/kernel/relu_basic.cu"
        ```

    === "Advanced"

        ```cpp title="relu_advanced"
        --8<-- "learning/cuda/kernel/relu_advanced.cu"
        ```

## leaky_relu

`leaky_relu` 保留正数输入，对负数输入乘以斜率系数：

$$
y_i = \begin{cases}
x_i, & x_i \ge 0 \\
\alpha x_i, & x_i < 0
\end{cases}
$$

它体现了 elementwise kernel 中常见的条件选择。

???+ note "leaky_relu 实现"

    === "Basic"

        ```cpp title="leaky_relu_basic"
        --8<-- "learning/cuda/kernel/leaky_relu_basic.cu"
        ```

    === "Advanced"

        ```cpp title="leaky_relu_advanced"
        --8<-- "learning/cuda/kernel/leaky_relu_advanced.cu"
        ```

## sigmoid

`sigmoid` 将输入压缩到 $(0, 1)$ 区间：

$$
y_i = \frac{1}{1 + e^{-x_i}}
$$

它的单元素计算量高于 ReLU，但线程之间仍没有数据依赖。

???+ note "sigmoid 实现"

    === "Basic"

        ```cpp title="sigmoid_basic"
        --8<-- "learning/cuda/kernel/sigmoid_basic.cu"
        ```

    === "Advanced"

        ```cpp title="sigmoid_advanced"
        --8<-- "learning/cuda/kernel/sigmoid_advanced.cu"
        ```

## silu

`SiLU` 是 sigmoid 与输入本身的乘积：

$$
y_i = x_i \cdot \sigma(x_i)
$$

它可以视为复合标量函数，但仍保持一输入一输出的 elementwise 形态。

???+ note "silu 实现"

    === "Basic"

        ```cpp title="silu_basic"
        --8<-- "learning/cuda/kernel/silu_basic.cu"
        ```

    === "Advanced"

        ```cpp title="silu_advanced"
        --8<-- "learning/cuda/kernel/silu_advanced.cu"
        ```

## swiglu

`SwiGLU` 对 value 与 gate 两路输入做门控变换：

$$
y_i = value_i \cdot \sigma(gate_i)
$$

它有两路输入，但每个输出位置仍只依赖同一位置的两个元素。

???+ note "swiglu 实现"

    === "Basic"

        ```cpp title="swiglu_basic"
        --8<-- "learning/cuda/kernel/swiglu_basic.cu"
        ```

    === "Advanced"

        ```cpp title="swiglu_advanced"
        --8<-- "learning/cuda/kernel/swiglu_advanced.cu"
        ```

## value_clipping

`value_clipping` 对数据的值进行上下边界截断：

$$
y_i = \max(\min(x_i, max\_val), min\_val)
$$

???+ note "value_clipping 实现"

    === "Basic"

        ```cpp title="value_clipping_basic"
        --8<-- "learning/cuda/kernel/value_clipping_basic.cu"
        ```

## color_inversion

`color_inversion` 针对图像处理应用执行255反转映射：

$$
y_i = 255 - x_i
$$

???+ note "color_inversion 实现"

    === "Basic"

        ```cpp title="color_inversion_basic"
        --8<-- "learning/cuda/kernel/color_inversion_basic.cu"
        ```

## 边界与局限

Elementwise kernel 通常受全局内存带宽限制。若单元素计算量很小，优化重点通常是 coalescing、packing 与减少 launch 次数，而不是复杂同步。

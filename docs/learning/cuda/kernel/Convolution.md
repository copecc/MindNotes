---
title: Convolution
tags:
  - CUDA
  - Kernel
  - Convolution
---

# Convolution

## 类型定位

卷积或 stencil 类 kernel 的关键特征是：相邻输出会重叠访问输入窗口，因此不同线程之间天然共享大量输入数据。这使得 shared memory staging 往往比在 elementwise 模式中的收益更明显。

## 优化方法

- Basic 版本让每个线程负责一个输出元素，并直接从 global memory 读取整个窗口。
- 将 block 覆盖的输入窗口搬入 shared memory，减少相邻输出的重复读取。
- 将卷积核系数放入 constant memory，使同一时刻所有线程读取同一 tap 时可以利用广播机制。
- 让单线程计算多个相邻输出，提高输入 tile 的复用率。

## 题目索引

- `convolution_1d`
- `convolution_3d`

## convolution_1d

`convolution_1d` 对一维输入序列执行 valid convolution。若输入为 $x$，卷积核为 $w$，kernel size 为 $K$，则输出长度为 $N - K + 1$，每个输出元素为：

$$
y_i = \sum_{j=0}^{K-1} x_{i+j} w_j
$$

这个公式说明相邻输出窗口会共享大部分输入元素。例如 $y_i$ 和 $y_{i+1}$ 的输入窗口只相差左右边界各一个元素。Basic 版本直接重复读取窗口，Advanced 版本用 shared memory 保存输入 tile，并把卷积核系数放入 constant memory。

???+ note "convolution_1d 实现"

    === "Basic"

        ```cpp title="convolution_1d_basic"
        --8<-- "learning/cuda/kernel/convolution_1d_basic.cu"
        ```

    === "Advanced"

        ```cpp title="convolution_1d_advanced"
        --8<-- "learning/cuda/kernel/convolution_1d_advanced.cu"
        ```

## convolution_3d

`convolution_3d` 操作涉及对三个维度 `(x, y, z)` 同时应用空间卷积：

$$
Y_{x,y,z} = \sum_{d} \sum_{r} \sum_{c} X_{x+c, y+r, z+d} \cdot W_{d, r, c}
$$

???+ note "convolution_3d 实现"

    === "Basic"

        ```cpp title="convolution_3d_basic"
        --8<-- "learning/cuda/kernel/convolution_3d_basic.cu"
        ```

## 边界与局限

当 kernel size 很大时，constant memory 不一定适合；当输入窗口与输出 tile 的关系复杂时，shared memory 大小和 occupancy 会形成新的约束。

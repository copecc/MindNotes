---
title: Mapping2D
tags:
  - CUDA
  - Kernel
  - Mapping2D
---

# Mapping2D

## 类型定位

二维映射类 kernel 把线程块中的二维坐标直接映射到矩阵、图像或二维网格中的 `(row, col)`。它与一维 elementwise 的区别不在计算一定更复杂，而在于 launch 和数据布局都需要同时处理行、列两个维度。

## 优化方法

- 使用 `dim3 threads(x, y)` 与 `dim3 blocks(grid_x, grid_y)` 表达二维工作区。
- 让连续的 `threadIdx.x` 对应连续列，匹配 row-major 存储下的合并访问。
- 对 transpose 这类重排问题，用 shared memory tile 把非连续写转换为更规整的写回路径。
- 对 shared memory tile 增加 padding，降低列方向访问时的 bank conflict。
- 对二维计数类问题，先做 block 内聚合，再减少全局 `atomicAdd` 次数。

## 题目索引

- `count_array_element2d`
- `matrix_transpose`
- `rgb_to_grayscale`
- `convolution_2d`

## Launch 方式

二维线程坐标通常由下式恢复：

$$
row = blockIdx.y \times blockDim.y + threadIdx.y
$$

$$
col = blockIdx.x \times blockDim.x + threadIdx.x
$$

## count_array_element2d

`count_array_element2d` 在二维数组中统计满足条件的元素数量：

$$
c = \sum_{r=0}^{H-1}\sum_{q=0}^{W-1} \mathbf{1}(x_{r,q} = target)
$$

Basic 版本中，每个线程负责一个二维坐标，命中条件后直接使用 `atomicAdd` 累加。Advanced 版本先在 block 内做局部计数，再用更少的全局 atomic 合并结果。

???+ note "count_array_element2d 实现"

    === "Basic"

        ```cpp title="count_array_element2d_basic"
        --8<-- "learning/cuda/kernel/count_array_element2d_basic.cu"
        ```

    === "Advanced"

        ```cpp title="count_array_element2d_advanced"
        --8<-- "learning/cuda/kernel/count_array_element2d_advanced.cu"
        ```

## matrix_transpose

`matrix_transpose` 把输入矩阵按行列互换写入输出：

$$
Y_{c,r} = X_{r,c}
$$

Basic 版本直接从 `(row, col)` 读取并写到 `(col, row)`。Advanced 版本先把 tile 搬入 shared memory，再交换坐标写回，并通过 `+1` padding 缓解 shared memory bank conflict。

???+ note "matrix_transpose 实现"

    === "Basic"

        ```cpp title="matrix_transpose_basic"
        --8<-- "learning/cuda/kernel/matrix_transpose_basic.cu"
        ```

    === "Advanced"

        ```cpp title="matrix_transpose_advanced"
        --8<-- "learning/cuda/kernel/matrix_transpose_advanced.cu"
        ```

## rgb_to_grayscale

`rgb_to_grayscale` 是一种典型的图像处理二维映射，它将通道数为 3 的 RGB 图像转换为单通道灰度图：

$$
Y_{r,c} = 0.299 \times R_{r,c} + 0.587 \times G_{r,c} + 0.114 \times B_{r,c}
$$

每个线程处理一个具体的像素坐标 `(row, col)`，读取对应的 RGB 连续分量进行加权后将结果输出到灰度通道：

???+ note "rgb_to_grayscale 实现"

    === "Basic"

        ```cpp title="rgb_to_grayscale_basic"
        --8<-- "learning/cuda/kernel/rgb_to_grayscale_basic.cu"
        ```

## convolution_2d

`convolution_2d` (即图像二维卷积与模糊等操作) 也是典型的二维映射问题。输出特征图中的一点会聚合原图中对应的领域值与滤波器的对应权重卷积计算。

$$
Y_{r, c} = \sum_{i} \sum_{j} X_{r+i, c+j} \times W_{i, j}
$$

Basic 实现中，每个二维坐标线程负责各自的领域乘加并进行越界保护：

???+ note "convolution_2d 实现"

    === "Basic"

        ```cpp title="convolution_2d_basic"
        --8<-- "learning/cuda/kernel/convolution_2d_basic.cu"
        ```

## 边界与局限

二维映射并不天然带来高复用。若访问模式只是简单坐标恢复，性能仍主要受全局内存访问方式支配；只有当读写模式发生重排时，shared memory tile 的收益才更明显。

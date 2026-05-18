---
title: Permutation
tags:
  - CUDA
  - Kernel
  - Permutation
---

# Permutation

## 类型定位

Permutation 类 kernel 的主要任务不是增加算术密度，而是改变数据位置映射。输出值通常直接来自输入，只是索引关系发生变化。

## 优化方法

- 保持至少一侧访存连续，例如连续读取输入或连续写回输出。
- 避免不必要的同步，因为简单 permutation 通常没有跨线程数据依赖。
- 只有当排列关系复杂、或需要在 tile 内重排时，才引入 shared memory 作为中间缓冲。

## 题目索引

- `reverse_array`
- `matrix_copy`
- `interleave`

## reverse_array

`reverse_array` 把输入数组按逆序写入输出。若输入长度为 $N$，可以写作：

$$
y_i = x_{N-1-i}
$$

实现上也可以让线程读取 $x_i$ 后写入 $y_{N-1-i}$。这个 kernel 的核心不是算术计算，而是索引关系变化；线程仍按连续 `idx` 启动，但输出地址由反向映射决定。这里展示的是 out-of-place 写法，输入和输出不能指向同一段内存。

???+ note "reverse_array 实现"

    === "Basic"

        ```cpp title="reverse_array_basic"
        --8<-- "learning/cuda/kernel/reverse_array_basic.cu"
        ```

## matrix_copy

`matrix_copy` 是将2D或展开的扁平矩阵做复制操作，没有复杂的数学运算：

$$
Y_{i, j} = X_{i, j}
$$

???+ note "matrix_copy 实现"

    === "Basic"

        ```cpp title="matrix_copy_basic"
        --8<-- "learning/cuda/kernel/matrix_copy_basic.cu"
        ```

## interleave

`interleave` 是典型的索引重排操作，将两个同等规模向量交叉混合：

$$
y_{2i} = a_i , \quad y_{2i+1} = b_i
$$

???+ note "interleave 实现"

    === "Basic"

        ```cpp title="interleave_basic"
        --8<-- "learning/cuda/kernel/interleave_basic.cu"
        ```

## 边界与局限

Permutation kernel 往往算术量很低，因此更容易完全受内存访问模式支配。它适合作为理解“索引重排”与“访存代价”关系的最小样例。

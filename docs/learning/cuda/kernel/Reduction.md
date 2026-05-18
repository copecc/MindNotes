---
title: Reduction
tags:
  - CUDA
  - Kernel
  - Reduction
---

# Reduction

## 类型定位

Reduction kernel 把一组输入收缩为更小的一组部分结果，或最终收缩为单个标量。它的主要难点不是单个线程如何恢复索引，而是 block 内部如何组织局部归约，以及多个 block 的局部结果如何合并。

这类 kernel 的典型结构是：每个线程先积累私有部分和，再在 block 内做树形归约，最后通过 atomic 或第二阶段 kernel 合并 block 结果。

## 优化方法

- 先做线程私有累加，减少进入 shared memory 的中间值数量。
- 使用 shared memory 做 block 内树形归约。
- 当归约范围缩小到一个 warp 时，使用 `__shfl_down_sync` 减少 shared memory 读写和同步。
- 用 block 级局部结果减少全局 atomic 的冲突压力。
- 对大输入使用分层 reduction，而不是单阶段 atomic 汇总。

## 题目索引

- `reduction`
- `count_array_element`
- `softmax`
- `dot_product`
- `mean_squared_error`

## reduction

`reduction` 把长度为 $N$ 的输入数组收缩为一个总和：

$$
s = \sum_{i=0}^{N-1} x_i
$$

Basic 版本使用 shared memory 做 block 内归约，并用 `atomicAdd` 合并 block 结果。Advanced 版本在最后阶段切换到 warp shuffle，减少同步与 shared memory 访问。

???+ note "reduction 实现"

    === "Basic"

        ```cpp title="reduction_basic"
        --8<-- "learning/cuda/kernel/reduction_basic.cu"
        ```

    === "Advanced"

        ```cpp title="reduction_advanced"
        --8<-- "learning/cuda/kernel/reduction_advanced.cu"
        ```

## count_array_element

`count_array_element` 统计数组中等于目标值的元素数量：

$$
c = \sum_{i=0}^{N-1} \mathbf{1}(x_i = target)
$$

它本质上是带谓词判断的整数 reduction。Basic 版本每次命中都执行一次全局 atomic；Advanced 版本先在 block 内计数，再由每个 block 发起一次 atomic。

???+ note "count_array_element 实现"

    === "Basic"

        ```cpp title="count_array_element_basic"
        --8<-- "learning/cuda/kernel/count_array_element_basic.cu"
        ```

    === "Advanced"

        ```cpp title="count_array_element_advanced"
        --8<-- "learning/cuda/kernel/count_array_element_advanced.cu"
        ```

## softmax

`softmax` 对输入向量执行归一化指数变换：

$$
y_i = \frac{e^{x_i - m}}{\sum_j e^{x_j - m}}, \quad m = \max_j x_j
$$

虽然输出形状仍是一组逐元素结果，但它依赖两次聚合：先求最大值，再求指数和。因此它在 kernel 模式上更接近 reduction 组合。

???+ note "softmax 实现"

    === "Basic"

        ```cpp title="softmax_basic"
        --8<-- "learning/cuda/kernel/softmax_basic.cu"
        ```

## dot_product

`dot_product` 生成两个向量的点积，是典型的双输入归约：

$$
s = \sum_{i=0}^{N-1} a_i \cdot b_i
$$

???+ note "dot_product 实现"

    === "Basic"

        ```cpp title="dot_product_basic"
        --8<-- "learning/cuda/kernel/dot_product_basic.cu"
        ```

## mean_squared_error

`mean_squared_error` 计算预测与真实标签之间的误差平方和：

$$
MSE = \frac{1}{N} \sum_{i=0}^{N-1} (p_i - l_i)^2
$$

Kernel 通常负责执行 Sum，然后在主机端求 Mean。

???+ note "mean_squared_error 实现"

    === "Basic"

        ```cpp title="mean_squared_error_basic"
        --8<-- "learning/cuda/kernel/mean_squared_error_basic.cu"
        ```

## 边界与局限

Reduction 常见瓶颈包括 atomic 冲突、shared memory 同步、以及跨 block 合并成本。输入很大时，单阶段 atomic 汇总通常不够理想，需要分层 reduction。

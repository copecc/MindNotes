---
title: Histogram
tags:
  - CUDA
  - Kernel
  - Histogram
---

# Histogram

## 类型定位

直方图 (Histogram) 操作涉及根据元素的值（被视作区间 `bin` 索引）对其各自的分类桶进行累计。此类操作属于密集散存操作 (Scatter)，不同线程向少数热点 `bin` 产生并发写入的概率较高，由此引发的数据冲突与并发写入开销是性能优化的核心。

## 优化方法

- Basic 版本：每个线程计算自己处理元素的 `bin` 并直接使用全局内存 (Global Memory) 上的 `atomicAdd`。当数据分布集中时，全局原子锁开销会导致严重的性能下降。
- Advanced 版本：利用 Shared Memory 缓冲冲突，先在 `block` 层面通过局部的 `atomicAdd` 构建私有直方图，最后再将这些局部统计结果跨 Block 安全合并到全局存储中。
- 大量短数据时还可引入寄存器缓存或者按 Warp 级别优化 atomic (如 `__reduce_add_sync` 等架构级指令)。

## 题目索引

- `histogram`

## histogram

针对给定的像素数组，统计每个像素值对应的频率数量。

$$
y_b = \sum_i \mathbf{1}(x_i = b)
$$

???+ note "histogram 实现"

    === "Basic"

        ```cpp title="histogram_basic"
        --8<-- "learning/cuda/kernel/histogram_basic.cu"
        ```

    === "Advanced"

        ```cpp title="histogram_advanced"
        --8<-- "learning/cuda/kernel/histogram_advanced.cu"
        ```

## 边界与局限

Shared Memory 的大小往往限制了直方图的最大 Bin 数量（例如当 Bin 个数以万计甚至更高时，无法将其完整存入 Shared Memory）。此时需回退到 Global Memory Atomic 或者执行对输入数据的划分预排序。

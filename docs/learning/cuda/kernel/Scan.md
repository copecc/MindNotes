---
title: Scan
tags:
  - CUDA
  - Kernel
  - Scan
---

# Scan

## 类型定位

Scan 或 prefix kernel 不只是做求和，而是为每个位置生成其前缀聚合结果。它与 reduction 的区别在于：reduction 只保留收缩结果，而 scan 需要保留所有位置的前缀状态。

在 CUDA 中，scan 通常先在 block 内完成树形扫描；当输入跨越多个 block 时，再把每个 block 的总和继续扫描，并把 block offset 回填到原输出上。

## 优化方法

- 小输入直接使用单 block shared memory scan。
- 使用 Blelloch scan 的 upsweep / downsweep 结构生成 exclusive prefix。
- 大输入拆成 block tiles，先生成每个 block 的局部 scan 与 `block_sums`。
- 对 `block_sums` 再执行第二层 scan，得到每个 block 的 offset。
- 最后把 block offset 回加到对应 block 的局部输出。

## 题目索引

- `exclusive_scan`
- `segmented_prefix_sum`

## exclusive_scan

`exclusive_scan` 为每个位置输出其左侧所有元素的前缀和，不包含当前位置本身：

$$
y_i = \sum_{j=0}^{i-1} x_j
$$

因此 $y_0 = 0$。Basic 版本展示单 block Blelloch scan。Advanced 版本展示多 block scan 的局部扫描和 offset 回加阶段；完整任意长度 scan 还需要继续扫描 `block_sums` 生成 `block_offsets`。

???+ note "exclusive_scan 实现"

    === "Basic"

        ```cpp title="exclusive_scan_basic"
        --8<-- "learning/cuda/kernel/exclusive_scan_basic.cu"
        ```

    === "Advanced"

        ```cpp title="exclusive_scan_advanced"
        --8<-- "learning/cuda/kernel/exclusive_scan_advanced.cu"
        ```

## segmented_prefix_sum

`segmented_prefix_sum` 是 Prefix Sum 的变体，通过 `flags` 数组判断是否开启新的一段序列。

遇到新序列标记时，重置累加逻辑。

???+ note "segmented_prefix_sum 实现"

    === "Basic"

        ```cpp title="segmented_prefix_sum_basic"
        --8<-- "learning/cuda/kernel/segmented_prefix_sum_basic.cu"
        ```

## 边界与局限

Scan 对同步结构比 reduction 更敏感，因为它必须保留前缀顺序信息。对于极大输入，仅靠单层 block offsets 往往不够，需要更完整的分层或递归设计。

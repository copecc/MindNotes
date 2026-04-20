---
title: 哈希
tags:
  - hot100
  - 哈希
---

# 哈希

哈希的核心作用是把“值”直接映射到“位置”或“桶”，从而把原本需要顺序扫描的信息查询压缩到均摊 $O(1)$。

在 LeetCode 的常见题型中，哈希通常承担三类工作：

- 记录某个值是否已经出现。
- 把一类具有相同特征的对象归到同一组。
- 快速判断某个值的前驱或后继是否存在。

## 两数之和

!!! problem "[1. 两数之和](https://leetcode.cn/problems/two-sum/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个整数数组 `nums` 和一个整数目标值 `target`，请你在该数组中找出和为目标值 `target` 的那两个整数，并返回它们的数组下标。

    你可以假设每种输入只会对应一个答案，并且你不能使用两次相同的元素。

这道题的关键在于把二元匹配关系改写成一次查找。

如果当前位置是 $i$，那么我们真正需要找的不是另一个值本身，而是它的补数：

$$
need = target - nums[i]
$$

因此在扫描数组时，可以维护一个哈希表，记录已经出现过的值及其位置。处理到 $nums[i]$ 时：

- 如果哈希表中已经存在 $need$，那么答案立刻成立。
- 如果不存在，就把当前值 $nums[i]$ 及其下标 $i$ 存入哈希表。

这样每个元素只会被处理一次，时间复杂度是 $O(n)$，额外空间复杂度是 $O(n)$。

???+ solution "1. 两数之和"

    ```cpp
    --8<-- "code/leetcode/hot100/L1.cpp"
    ```

## 字母异位词分组

!!! problem "[49. 字母异位词分组](https://leetcode.cn/problems/group-anagrams/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给你一个字符串数组 `strs`，请你将字母异位词组合在一起。可以按任意顺序返回结果列表。

如果两个字符串互为字母异位词，那么它们在重排后会得到完全相同的字符序列。因此可以把“排序后的字符串”当作哈希键。

设原字符串为 $s$，把它排序后得到标准形：

$$
key(s) = \operatorname{sort}(s)
$$

若两个字符串 $s_1, s_2$ 满足

$$
key(s_1) = key(s_2)
$$

则它们一定属于同一组异位词。

于是只需用哈希表维护：

$$
key \longrightarrow \text{该组所有原字符串}
$$

遍历结束后，把哈希表中的所有桶取出即可。

这个做法的关键不是“哈希字符串本身”，而是“先构造能唯一表示组别的特征，再把这个特征作为哈希键”。

???+ solution "49. 字母异位词分组"

    ```cpp
    --8<-- "code/leetcode/hot100/L49.cpp"
    ```

## 最长连续序列

!!! problem "[128. 最长连续序列](https://leetcode.cn/problems/longest-consecutive-sequence/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个未排序的整数数组 `nums`，找出数字连续的最长序列的长度。

    你必须设计并实现时间复杂度为 $O(n)$ 的算法解决此问题。

这道题如果先排序，可以直接做，但时间复杂度会变成 $O(n \log n)$。题目要求 $O(n)$，因此需要用哈希集合直接判断某个值是否存在。

把所有数字放入哈希集合后，若某个数 $x$ 不是连续段的起点，那么一定存在它的前驱：

$$
x - 1 \in S
$$

只有当

$$
x - 1 \notin S
$$

时，$x$ 才是某一段连续序列的起点。此时再不断检查

$$
x+1,\ x+2,\ x+3,\ \dots
$$

是否在集合中，就能得到以 $x$ 为起点的连续段长度。

这样每个元素至多被当作某个连续段中的一部分访问一次，整体仍然是线性复杂度。核心优化点是：只从起点出发扩展，而不是从每个元素都重复扩展。

???+ solution "128. 最长连续序列"

    ```cpp
    --8<-- "code/leetcode/hot100/L128.cpp"
    ```

---
title: 元素计数
tags:
  - Element Count
  - Unique Count
  - Balanced Frequency
---

# 元素计数

元素计数是区间查询中的一个经典问题，常见的问题包括统计区间内不同元素的个数、不同元素出现相同次数的最长子区间等。

## 不同元素计数

在区间查询中，统计区间内不同元素的个数是一个常见的问题。

解决这类问题的核心方法是：

在处理区间查询时，按右端点从小到大排序，然后逐步扩展右端点。当发现相同元素时，撤销其上一次出现的位置，从而确保每个元素在当前区间内只被计数一次。因为是按右端点排序的，所以被撤销的操作不会影响查询结果。

???+ note "[HH 的项链](https://www.luogu.com.cn/problem/P1972){target=_blank}"

    给定一个长度为 $n$ 的序列和 $m$ 个查询，每个查询包含两个整数 $l$ 和 $r$，计算子区间 $[l, r]$ 中不同整数的个数。

    === "离线查询"

        ```cpp
        --8<-- "code/Trick/ElementCount/P1972_1.cpp"
        ```

    === "可持久化线段树"

        !!! hint "版本"
        
            可持久化线段树的版本通过为每个位置创建一个新的版本来记录元素的出现情况。当一个元素再次出现时，更新其上一次出现位置的节点，将该位置的值设为0，从而确保每个元素在当前区间内只被计数一次。

        !!! warning "TLE"
        
            使用可持久化线段树版本可以实现在线查询，扩展到新的右端点时就创建一个新的版本。然而，这样实现在会 **TLE** 两个测试点。因此，对于该问题还是使用离线查询。

        ```cpp
        --8<-- "code/Trick/ElementCount/P1972_2.cpp"
        ```

## 频次平衡子数组

频次平衡类问题通常涉及寻找一个子区间满足某种计数平衡条件。

对这类问题通常每个位置转化为一个状态，并在前缀状态上使用哈希表寻找匹配的前缀，从而确定满足条件的子区间。

形式化地：

$$
\text{feature}[i] = f(\text{count}_1(i), \text{count}_2(i), \dots)
$$

若存在 $l < r$ 使得

$$
\text{feature}[r] = \text{feature}[l],
$$

则说明区间 $(l+1, r]$ 是一个平衡区间。

于是我们只需记录每个状态第一次出现的位置，用于计算最长距离。

???+ note "[连续数组](https://leetcode.cn/problems/contiguous-array/description/){target=_blank}"

    给定一个二进制数组 $nums$ , 找到含有相同数量的 $0$ 和 $1$ 的最长连续子数组，并返回该子数组的长度。

    !!! hint "前缀和"

        将数组中的 $0$ 替换为 $-1$，然后计算前缀和。对于任意两个前缀和相等的位置 $i$ 和 $j$，子数组 $nums[i+1 \ldots j]$ 中 $0$ 和 $1$ 的数量相同。通过记录每个前缀和第一次出现的位置，可以在遍历数组时计算出最长的满足条件的子数组长度。

    ```cpp
    --8<-- "code/Trick/ElementCount/contiguous-array.cpp"
    ```

??? note "[每个元音包含偶数次的最长子字符串](https://leetcode.cn/problems/find-the-longest-substring-containing-vowels-in-even-counts/description/){target=_blank}"

    给定一个字符串 $s$ ，找到含有每个元音字母（a, e, i, o, u）出现偶数次的最长子字符串，并返回该子字符串的长度。

    !!! hint "前缀和"

        将每个元音字母的出现次数视为一个状态，使用一个整数表示每个状态的奇偶性。通过记录每个状态第一次出现的位置，可以在遍历字符串时计算出最长的满足条件的子字符串长度。

    ```cpp
    --8<-- "code/Trick/ElementCount/find-the-longest-substring-containing-vowels-in-even-counts.cpp"
    ```

??? note "[最长的平衡子串 II](https://leetcode.cn/problems/longest-balanced-substring-ii/description/){target=_blank}"

    给你一个只包含字符 'a'、'b' 和 'c' 的字符串 $s$。如果一个子串中所有不同字符出现的次数都相同，则称该子串为平衡子串。请返回 $s$ 的最长平衡子串的长度。

    !!! hint "二维前缀差"

        分别讨论只有一种字符、两种字符和三种字符的情况。对于两种字符的情况，可以使用类似于连续数组的方法，计算两个字符出现次数的差值，并记录每个差值第一次出现的位置。对于三种字符的情况，可以使用一个二维状态表示三个字符的出现次数差值，并记录每个状态第一次出现的位置。

    ```cpp
    --8<-- "code/Trick/ElementCount/longest-balanced-substring-ii.cpp"
    ```

??? note "[最长平衡子数组 II](https://leetcode.cn/problems/longest-balanced-subarray-ii/description/){target=_blank}"

    给你一个整数数组 $nums$，如果子数组中不同偶数的数量等于不同奇数的数量，则称该子数组是平衡的。

    返回最长平衡子数组的长度。

    !!! hint annotate "线段树 + 前缀和"

        使用线段树维护前缀和信息。遍历数组时，计算当前前缀和，并根据当前元素的奇偶性更新线段树。对于每个前缀和，使用线段树查询最早出现该前缀和的位置，从而计算出最长的平衡子数组长度。

        前缀和的定义为：对于位置 $i$，若 $nums[i]$ 为偶数，则前缀和加 $1$，否则减 $1$。这样，前缀和相等的位置表示在该区间内偶数和奇数的数量相等(1)。由于每次更新时只会加减 $1$，因此通过维护线段树的最小值和最大值，可以高效地查询前缀和的位置。

        注意由于线段树的区间是从1开始的，因此在处理前缀和时需要进行适当的偏移。

    1. 如果区间的 $min \leq target \leq max$，那必然存在某个位置 $pos$ 等于 $target$。这是因为每次更新只会加减 $1$，所以区间内的值是连续的。
        

    ```cpp
    --8<-- "code/Trick/ElementCount/longest-balanced-subarray-ii.cpp"
    ```
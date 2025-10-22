---
title: MEX
tags:
  - 最小缺失值
  - MEX
---

# MEX

$\text{MEX}$（$\text{Minimum Excludant}$）指的是一个集合中没有出现的最小非负整数。换句话说，$\text{MEX}$ 是从 $0$ 开始、不属于给定的集合的最小整数。  
$\text{MEX}(S) = \min\{x \geq 0 \mid x \notin S\}$

!!! tip "$\text{MEX}$与博弈论"
    在博弈论中，$\text{MEX}$ 常用于计算 $\text{Grundy}$ 数（格兰迪数），以判断当前局势是否为必胜态

$\text{MEX}$ 的一些性质：  

1. 最大可能值: 对于一个包含 $n$ 个非负整数的集合 $S$，$\text{MEX}(S)$ 的最大可能值是 $n$  
2. 最小可能值: 对于任何非空集合 $S$，$\text{MEX}(S)$ 的最小可能值是 $0$  
3. 单调性: 如果集合 $A$ 是集合 $B$ 的子集，那么 $\text{MEX}(A) ≤ \text{MEX}(B)$。这意味着向集合中添加一个元素可能会导致 $\text{MEX}$ 值的增加，但不会减少  
4. 子集性质: 如果 $S$ 是一个包含 $n$ 个非负整数的集合，并且 $\text{MEX}(S) = m$，那么集合 $\{0, 1, 2, \ldots, m-1\}$ 必须是 $S$ 的子集  
5. 补集性质: 如果 $S$ 是全集 $U = \{0, 1, 2, \ldots, m\}$ 的子集，那么 $\text{MEX}(S)$ 是补集 $U \setminus S$ 的最小元素  

???+ note "MEX的计算"
    === "数组辅助"
        时间复杂度 $O(n)$，空间复杂度 $O(n)$。

        ```cpp
        int mex(const std::vector<int> &nums) {
          int n = nums.size();
          // n个数的数组，mex最大可能是n
          std::vector<bool> present(n + 1, false);
          for (int num : nums) {  // 标记数组中出现过的数字
            if (num <= n) { present[num] = true; }
          }
          for (int i = 0; i <= n; ++i) {  // 找到第一个未出现的数字
            if (!present[i]) { return i; }
          }
          return n;  // 如果0到n-1都出现了，返回n
        }
        ```

    === "负号标记"
        时间复杂度 $O(n)$，空间复杂度 $O(1)$。

        ```cpp
        int mex(std::vector<int> &nums) {
          int n = nums.size();
          for (int i = 0; i < n; i++) {
            if (nums[i] <= 0) { nums[i] = n + 1; }  // 将负数和零替换为n+1
          }
          for (int i = 0; i < n; i++) {
            // 使用负号标记出现过的数字
            if (abs(nums[i]) <= n && nums[abs(nums[i]) - 1] > 0) { nums[abs(nums[i]) - 1] *= -1; }
          }
          for (int i = 0; i < n; i++) {
            if (nums[i] > 0) { return i + 1; }  // 第一个正数的索引+1即为mex
          }
          return n + 1;
        }
        ```

    === "双指针"
        双指针方法利用了数组的特性，将有效数字放在左边，垃圾数字放在右边。时间复杂度 $O(n)$，空间复杂度 $O(1)$。

        ```cpp
        int mex(std::vector<int> &nums) {
          // left 左边的区域已经放好 1~left 的数字
          int left = 0;
          // [right....]垃圾区
          int right = nums.size();
          while (left < right) {
            // 如果当前位置正好是 left+1，扩展 mex 区域
            if (nums[left] == left + 1) {
              left++;
            } else if (nums[left] <= left || nums[left] > right || nums[nums[left] - 1] == nums[left]) {
              // 如果当前数字无效（<= left、> right、重复），丢到垃圾区
              --right;
              swap(nums[left], nums[right]);
            } else {
              // 把当前数字放到它应该在的位置
              swap(nums[left], nums[nums[left] - 1]);
            }
          }
          return left + 1;
        }
        ```

!!! hint "动态查询$\text{MEX}$"
    动态更新查询区间$\text{MEX}$需要权值线段树维护

??? note "[Rmq Problem / mex](https://www.luogu.com.cn/problem/P4137){target=_blank}"

    有一个长度为 $n$ 的数组 $a_1,a_2,\dots,a_n$。 有 $m$ 个查询，每个查询给出两个整数 $l$ 和 $r$，要求你找出子区间 $[l,r]$ 内的 $\text{MEX}$。

    ??? hint

        维护每一个权值在原数组中最后一次出现的下标。对于查询操作 $[l,r]$，如果某个权值的最后出现位置小于 $l$，则该权值不在区间 $[l,r]$ 内出现。于是维护一个线段树，节点保存该区间内所有权值的最后出现位置的最小值。然后可以通过线段树二分查找小于 $l$ 的最小权值，即为 $\text{MEX}$。

        由于 $n$ 个数的 $\text{MEX}$ 最大可能为 $n$，所以对于大于 $n$ 的数，可以直接将其视为不存在。对于每个区间右端点 $r$，只需要将 $a_r$ 插入到线段树中，并更新其最后出现位置为 $r$。

    ```cpp
    --8<-- "code/Math/MEX/P4137.cpp"
    ```

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
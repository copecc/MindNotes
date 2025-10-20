---
title: 置换环
tags:
  - 置换环
  - Permutation Cycle
---

# 置换环

给定集合 $S=\{1,\dots,n\}$，置换是 $S$ 上的双射。任一置换可以唯一（除循环顺序外）分解为若干两两不相交的环（$\text{cycle}$）。  
在数组排序问题中，常把当前数组与目标数组的下标映射视为一个置换：若元素从下标 $i$ 应放到下标 $P(i)$，则下标映射 $i\mapsto P(i)$ 是一个置换，环即为索引循环。

!!! abstract "环的性质与最少交换次数证明"

    设一个置换分解为 $r$ 个不相交环，环的长度分别为 $k_1, k_2, \ldots, k_r$，则有 $\sum_{i=1}^r k_i = n$。  
    设一个环长度为 $k$。要把环中的元素放回正确位置，最少需要 $k-1$ 次交换：每次把一个不在位的元素直接交换到它应在的位置，能把环中一个元素固定到位，重复 $k-1$ 次后剩下一个元素自然在位。  
    若置换分解为 $r$ 个不相交环且总元素数为 $n$，则最小交换次数为

    $$
    \sum_{i=1}^r (k_i-1) = \Big(\sum_{i=1}^r k_i\Big) - r = n - r.
    $$

    另一种证明思路：每次交换至多将环数增加 $1$，从初始 $r$ 个环增加到 $n$ 个（每个元素自成环）需 $n-r$ 次交换，且可以达到，因此下界可达。

???+ note "置换环与最少交换次数"

    给定一个长度为 $n$ 的数组 $nums$，数组元素互不相同且均在 $[1,n]$ 之间。求将数组排序所需的最少交换次数。  

    ```cpp
    #include <algorithm>
    #include <cstdint>
    #include <vector>
    using namespace std;

    // 返回使数组非降序所需的最少交换次数
    int64_t min_swaps_to_sort(const vector<int64_t>& nums) {
      int64_t n = nums.size();
      vector<pair<int64_t, int64_t>> v(n);
      for (int64_t i = 0; i < n; ++i) { v[i] = {nums[i], i}; }
      sort(v.begin(), v.end());  // 按值排序（相同值按原下标区分）
      vector<int64_t> to(n);
      for (int64_t j = 0; j < n; ++j) { to[v[j].second] = j; }
      vector<int64_t> vis(n, 0);
      int64_t ans = 0;
      for (int64_t i = 0; i < n; ++i) {
        if (vis[i]) { continue; }
        int64_t cur = i, len = 0;
        while (!vis[cur]) {
          vis[cur] = 1;
          cur      = to[cur];
          ++len;
        }
        if (len > 0) ans += len - 1;
      }
      return ans;
    }
    ```
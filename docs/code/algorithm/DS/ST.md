---
title: ST表
tags:
  - ST表
  - Sparse Table
  - 倍增
---

# ST表

$ST$ 表（$\text{Sparse Table}$）是一种用于在静态数组上高效处理区间查询的算法数据结构。  
$ST$ 表适用于幂等（$\text{idempotent}$）操作，而区间和（$sum$）等非幂等操作则不适用。  
$ST$ 表的核心思想是预处理数组的多个区间，使得任何查询都可以通过合并预处理的区间来快速得到结果。  
$ST$ 表是一种典型的倍增（$\text{Doubling}$）技术应用。  

!!! example "应用场景"

    - 区间最值查询（$\text{Range Minimum/Maximum Query}$）
    - 区间最大公约数查询（$\text{Range GCD Query}$）
    - 其他满足结合律的幂等操作（如 $\min$、$\max$、$\gcd$、$\text{bitwise\_and}$、$\text{bitwise\_or}$ 等）

!!! note "$ST$ 表模板"

    ```cpp
    struct ST {
      int n, m;

      vector<int> log2;  // floor(log2(i)), 也可以用 31 - __builtin_clz(i)
      vector<vector<int>> st;
      std::function<int(int, int)> func;  // 结合律运算函数, 如 min, max, gcd

      ST(const vector<int> &nums, const std::function<int(int, int)> &f)
          : n(nums.size()), log2(n + 1), func(f) {
        for (int i = 2; i <= n; i++) { log2[i] = log2[i / 2] + 1; }
        m = log2[n] + 1;  // m = log2[n] + 1 = 32 - __builtin_clz(n) (1)
        st = vector<vector<int>>(n, vector<int>(m));
        // 预处理 ST 表, O(nlogn)
        for (int i = 0; i < n; i++) { st[i][0] = nums[i]; }
        for (int j = 1; j < m; j++) {
          for (int i = 0; i + (1 << j) - 1 < n; i++) {  // 确保区间右端点不会越界 (2) (3)
            st[i][j] = func(st[i][j - 1], st[i + (1 << (j - 1))][j - 1]);
          }
        }
      }

      // 查询区间 [l, r] 的值，0 <= l <= r < n (4)
      int query(int l, int r) {
        int j = log2[r - l + 1];  // 计算不超过区间长度的最大2的幂次 (5)
        return func(st[l][j], st[r - (1 << j) + 1][j]);
      }
    };
    ```

    1. 需要的列数 $m$ 由 $n$ 决定，最多需要 $\lfloor log_{2}(n) \rfloor + 1$ 列 ： $2^m = 2^{\lfloor log_{2}(n) \rfloor + 1} > n$
    2. 整个区间 $[i, i + 2^{j} - 1]$ 可以分解为两个子区间:  
        $[i, i + 2^{j - 1})$ 和 $[i + 2^{j - 1}, i + 2^{j})$  
        这两个子区间的长度均为 $2^{j - 1}$
    3. 另一种常见处理方法，根据前进 $2^{j-1}$ 步再前进 $2^{j-1}$ 步来合并, 见[LCA](../Tree/LCA.md#倍增法)  
        `for (int i = 0; i < n; i++) { st[i][j] = st[st[i][j - 1]][j - 1]; }`
    4. 区间 $[l, r]$ 可以分解为两个子区间:  
        $[l, l + 2^{j} - 1]$ 和 $[r - 2^{j} + 1, r]$  
        这两个子区间覆盖了整个区间 $[l, r]$ （可能重叠，但是幂等性不会影响结果），并且长度均为 $2^{j}$。  
    5. 也可以用 j = 31 - __builtin_clz(r - l + 1);


??? note "[Balanced Lineup G](https://www.luogu.com.cn/problem/P2880){target=_blank}"
    
    给定一个长度为 $n$ 的整数序列 $a_1, a_2, \ldots, a_n$，以及 $m$ 个查询，每个查询包含两个整数 $l$ 和 $r$，要求你在子序列 $a_l, a_{l+1}, \ldots, a_r$ 中找到最大值与最小值的差值。

    ```cpp
    --8<--  "code/DS/ST/P2880.cpp"
    ```

  

    
---
title: 离散化
tags:
  - discretization
---

# 离散化

离散化（$\text{Discretization}$）将较大范围的数值映射到较小范围的整数，通常是从 $1$ 开始的连续整数。  
离散化之后值域缩小到和数据规模相当的范围内，便于使用基于数组下标的数据结构（如树状数组、线段树等）进行高效处理。

!!! tip "离散化的应用场景"
    离散化的前提是只关心数值的相对大小关系，而不关心具体数值。  

???+ note "离散化"

    === "排序、去重"
        时间复杂度 $O(n \log n)$, 空间复杂度 $O(n)$

        ```cpp
        #include <algorithm>
        #include <vector>
        using namespace std;

        vector<int> discretize(const vector<int> &nums) {
          vector<int> sorted_nums = nums;
          sort(sorted_nums.begin(), sorted_nums.end());
          sorted_nums.erase(unique(sorted_nums.begin(), sorted_nums.end()), sorted_nums.end());
          return sorted_nums;
        }

        // 返回 x 在离散化后数组中的排名（从1开始）
        int get_rank(const vector<int> &sorted_nums, int x) {
          return lower_bound(sorted_nums.begin(), sorted_nums.end(), x) - sorted_nums.begin() + 1;
        }
        ```
    
    === "无重复元素"
        时间复杂度 $O(n \log n)$，空间复杂度 $O(n)$。

        ```cpp
        #include <algorithm>
        #include <vector>
        using namespace std;

        vector<int> discretize(const vector<int> &nums) {
          int n = nums.size();
          vector<int> index(n);
          // 映射至[0,n)
          iota(index.begin(), index.end(), 0);
          sort(index.begin(), index.end(), [&](int i, int j) { return nums[i] < nums[j]; });
          vector<int> rank(n);
          for (int i = 0; i < n; ++i) { rank[index[i]] = i; }
          return rank;
        }
        ```
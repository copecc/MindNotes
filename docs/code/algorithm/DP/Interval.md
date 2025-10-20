---
title: 区间 DP
tags:
  - 区间型动态规划
  - Interval DP
---

# 区间型动态规划

区间型动态规划（$\text{Interval DP}$）是动态规划的一种特殊类型，通常用于解决与区间相关的问题。其核心思想是将问题划分为若干个子区间，通过递推关系来求解整个区间的问题。

区间类动态规划展开的常见方式：

1. 基于两侧端点讨论的可能性展开
2. 基于范围上划分点的可能性展开

令状态 $f(i, j)$ 表示将下标位置 $i$ 到 $j$ 的所有元素合并能获得的价值的最大值，则状态转移方程为：

$$
f(i, j) = \max_{i \leq k < j} \{f(i, k) + f(k+1, j) + \text{cost}(i, j)\}
$$

其中，$\text{cost}(i, j)$ 表示将区间 $[i, j]$ 合并所获得的价值。

## 基于两侧端点讨论

这种方式通常用于问题中需要考虑区间两端的元素时，可以通过选择区间的左端点或右端点来进行状态转移。

???+ note "[让字符串成为回文串的最少插入次数](https://leetcode.cn/problems/minimum-insertion-steps-to-make-a-string-palindrome/description/){target=_blank}"

    给你一个字符串 $s$ ，每一次操作你都可以在字符串的任意位置插入任意字符。请你返回让 $s$ 成为回文串的最少操作次数。

    === "记忆化搜索"

        ```cpp
        class Solution {
         public:
          int minInsertions(string s) {
            int n = s.size();
            vector<vector<int>> memo(n, vector<int>(n, -1));
            auto dfs = [&](auto &&self, int i, int j) {
              if (i >= j) { return 0; }
              // 记忆化搜索
              if (memo[i][j] != -1) { return memo[i][j]; }
              int res = 0;         // 空串或单字符已经是回文，不需要插入
              if (s[i] == s[j]) {  // 两端字符相同，不需要插入
                res = self(self, i + 1, j - 1);
              } else {  // 插入一个字符，使得两端字符相同
                res = min(self(self, i + 1, j), self(self, i, j - 1)) + 1;
              }
              memo[i][j] = res;  // 记忆化
              return res;
            };
            return dfs(dfs, 0, n - 1);
          }
        };
        ```

    === "位置依赖"

        ```cpp
        class Solution {
         public:
          int minInsertions(string s) {
            int n = s.size();
            vector<vector<int>> dp(n, vector<int>(n, 0));
            // dp[i][j] 依赖 dp[i+1][j-1], dp[i+1][j], dp[i][j-1]
            for (int i = n - 1; i >= 0; --i) {
              for (int j = i + 1; j < n; ++j) {
                if (s[i] == s[j]) {  // 两端字符相同，不需要插入
                  dp[i][j] = dp[i + 1][j - 1];
                } else {  // 插入一个字符，使得两端字符相同
                  dp[i][j] = min(dp[i + 1][j], dp[i][j - 1]) + 1;
                }
              }
            }
            return dp[0][n - 1];
          }
        };
        ```
    
    === "空间优化"

        ```cpp
        class Solution {
         public:
          int minInsertions(string s) {
              int n = s.size();
              vector<int> dp(n, 0);
              // dp[i][j] 依赖 dp[i+1][j-1], dp[i+1][j], dp[i][j-1], 即左下角, 左边, 下边 
              for (int i = n - 1, left_down = 0; i >= 0; --i) {
                left_down = 0;  // 每次更新左下角
                for (int j = i + 1; j < n; ++j) {
                  int back_up = dp[j];  // 更新前的 dp[i+1][j]
                  if (s[i] == s[j]) {  // 两端字符相同，不需要插入
                    dp[j] = left_down;
                  } else {  // 插入一个字符，使得两端字符相同
                    dp[j] = min(dp[j], dp[j - 1]) + 1;
                  }
                  left_down = back_up;
                }
              }
              return dp[n-1];
            }
        };
        ```

## 范围上划分点讨论

这种方式通常用于问题中需要考虑区间内部元素时，可以通过选择区间内的某个划分点来进行状态转移。

!!! tip "空间优化"

    基于范围上划分点讨论的区间 $\text{DP}$ 通常不容易进行空间优化，因为状态转移依赖于多个子区间的结果，难以通过滚动数组等技术来减少空间复杂度。

???+ note "[切棍子的最小成本](https://leetcode.cn/problems/minimum-cost-to-cut-a-stick/description/){target=_blank}"

    给你一根长度为 $n$ 的绳子，绳子上有一些需要切割的位置，切割一次的成本为当前绳子的长度。请你计算出将绳子切割完所需的最小成本。

    === "记忆化搜索"

        ```cpp
        class Solution {
         public:
          int minCost(int n, vector<int> &cuts) {
            int m = cuts.size();
            vector<vector<int>> memo(m, vector<int>(m, -1));

            sort(cuts.begin(), cuts.end());
            auto dfs = [&](auto &&self, int left, int right) {
              if (left > right) { return 0; }
              // 记忆化搜索
              if (memo[left][right] != -1) { return memo[left][right]; }
              // 计算当前区间的实际长度(切割一次的成本)
              int cost = (right == m - 1 ? n : cuts[right + 1]) - (left == 0 ? 0 : cuts[left - 1]);
              int res  = INT_MAX;
              // 枚举所有可能的划分点
              for (int i = left; i <= right; ++i) {
                res = min(res, self(self, left, i - 1) + self(self, i + 1, right) + cost);
              }
              memo[left][right] = res;  // 记忆化
              return res;
            };
            return dfs(dfs, 0, m - 1);
          }
        };
        ```

    === "位置依赖"

        ```cpp
        class Solution {
         public:
          int minCost(int n, vector<int> &cuts) {
            int m = cuts.size();

            sort(cuts.begin(), cuts.end());
            vector<vector<int>> dp(m, vector<int>(m, 0));
            // dp[left][right] 依赖 dp[left][i-1], dp[i+1][right] (left<=i<=right)
            for (int left = m - 1; left >= 0; --left) {
              for (int right = left; right < m; ++right) {
                // 计算当前区间的实际长度
                int cost        = (right == m - 1 ? n : cuts[right + 1]) - (left == 0 ? 0 : cuts[left - 1]);
                dp[left][right] = INT_MAX;
                // 枚举所有可能的划分点
                for (int i = left; i <= right; ++i) {
                  int left_cost   = (i > left) ? dp[left][i - 1] : 0;
                  int right_cost  = (i < right) ? dp[i + 1][right] : 0;
                  dp[left][right] = min(dp[left][right], left_cost + right_cost + cost);
                }
              }
            }
            return dp[0][m - 1];
          }
        };
        ```

!!! tip "基于区间长度的遍历顺序"

    在区间 $\text{DP}$ 中，必须按照区间长度从小到大遍历以保证依赖顺序。而在每个固定区间内部，枚举划分点 $i$ 的方向（如从左到右）不影响结果。
    
    因此从记忆化搜索转为递推时，只需保证外层“区间长度递增”，内层枚举方式保持不变即可。
---
title: 数位 DP
tags:
  - 数位动态规划
  - Digit DP
---

# 数位动态规划

数位动态规划（$\text{Digit DP}$）用于解决与数字的位数、数位相关的问题。其核心思想是通过对数字的每一位进行状态定义和转移，从而求解整个数字的问题。

数位是指把一个数字按照个、十、百、千等等一位一位地拆开，关注它每一位上的数字。如果拆的是十进制数，那么每一位数字都是 $[0,9]$，其他进制可类比十进制。

数位 $\text{DP}$ 问题一般具有这几个特征：

1. 要求统计满足一定条件的数的数量，即最终目的为计数
2. 这些条件经过转化后可以使用数位的思想去理解和判断
3. 输入会提供一个数字区间（有时也只提供上界）来作为统计的限制
4. 上界很大（比如 $10^{18}$），暴力枚举验证会超时

一般解决这类问题的方法是通过记忆化搜索，即通过递归的方式枚举每一位数字，同时记录当前枚举的位数、是否受限制、是否已经填了数字等状态。这样可以通过备忘录记录已经计算过的状态，避免重复计算。

1. 备忘录 $memo$：记录已经计算过的状态，避免重复计算。初始化为-1，表示没有计算过
2. $DFS$ 函数：递归枚举每一位数字，同时记录当前枚举的位数、是否受限制、是否已经填了数字等状态。通过备忘录记录已经计算过的状态，避免重复计算。各个参数的含义：  
    - $i$：当前枚举的第几位。如果 $i$ 等于数字的位数，表示已经枚举完了
    - $is\_limit$：当前枚举数是否受限制。如果受限制表示当前枚举的数字不能超过上界。比如，如果上界是 $1234$，那么在枚举第一位的时候，只能枚举 $[0,1]$，不能枚举 $[2,9]$；如果第一位枚举了 $1$，那么第二位只能枚举 $[0,2]$，不能枚举 $[3,9]$
    - $is\_num$：当前是否已经填了数字，受前导 $0$ 限制。如果前导 $0$ 有影响，需要去掉该参数，或第一次调用时设置为 $true$
    - 添加其他参数：如 $pre$ 表示前一位填的数字，$mask$ 表示已经使用的数字，$diff$ 表示奇偶位数差异（可能为负数需要加上偏移量），$mod$ 表示余数

备忘录 $memo$ 的维度与可变参数个数有关，$is\_limit$ 和 $is\_num$ 无需记忆化，因为使用 $memo$ 时这两个参数必定为 $false$ 和 $true$。

在问题求解时，一般将 $[l, r]$ 转换为 $[0, r]$ 和 $[0, l-1]$，然后分别求解，最后相减即可。  
如果 $l-1$ 也不适合（如 $l=0$ 或非常大的数字），则直接求解 $[0, l]$，然后相减并且对 $l$ 进行特殊的 $check$ 处理，判断是否满足条件。

???+ note "模版"

    ```cpp
    int64_t digit_dp(const string &s) {
      int64_t m = s.length();

      vector<int64_t> memo(m, -1);  // -1 表示没有计算过

      using dfs_type = function<int64_t(int64_t, bool, bool)>;
      dfs_type dfs   = [&](int64_t i, bool is_limit, bool is_num, ...) -> int64_t {
        if (i == m) { return is_num; }  // is_num 为 true 表示得到了一个合法数字
        if (!is_limit && is_num && memo[i] != -1) { return memo[i]; }
        int64_t res = 0;
        // 可以跳过当前数位
        if (!is_num) { res = dfs(i + 1, false, false); }

        int64_t up = is_limit ? s[i] - '0' : 9;  // 当前位可填的最大数字
        for (int64_t d = 1 - is_num; d <= up; ++d) {  // 枚举要填入的数字 d
          // if 中填限制条件
          if (true) { res = res + dfs(i + 1, is_limit && d == up, true); }
        }
        if (!is_limit && is_num) { memo[i] = res; }
        return res;
      };
      // 开始时要约束，所以is_limit为true
      return dfs(0, true, false);
    }

    int64_t solve(int64_t low, int64_t high) {
      string low_str  = to_string(low - 1);
      string high_str = to_string(high);
      return digit_dp(high_str) - digit_dp(low_str);
    }
    ```

??? note "[数字计数](https://www.luogu.com.cn/problem/P2602){target=_blank}"

    给定两个正整数 $a$ 和 $b$，求在 $[a,b]$ 中的所有整数中，每个数码（$digit$）各出现了多少次。

    ```cpp
    --8<-- "code/DP/Digit/P2602.cpp"
    ```

??? note "[MYQ10 - Mirror Number](https://www.luogu.com.cn/problem/SP10649){target=_blank}"

    求 $[a,b]$ 中镜像回文的个数。镜像回文是指上下对称，左右对称的数字。显然：镜像回文由 $0,1,8$ 构成。

    ```cpp
    --8<-- "code/DP/Digit/SP10649.cpp"
    ```
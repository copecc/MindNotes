---
title: 组合数
tags:
  - 组合数
  - Combination
---

# 组合数

## 组合数的计算

组合数（$\text{Combination}$）是指从 $n$ 个不同元素中任取 $k$ 个元素的选法数，记作 $C(n, k)$ 或 $\binom{n}{k}$。  
组合数的计算公式为：$C(n, k) = \frac{n!}{k! \cdot (n-k)!}, 0 \leq k \leq n$  
重要性质：  

1. $C(n, k) = C(n, n-k)$（对称性）  
2. $C(n, k) = C(n-1, k-1) + C(n-1, k)$（递推关系）  
3. $\sum_{k=0}^{n} C(n, k) = 2^n$（二项式定理）  
4. $C(n, k) = \frac{n}{k} \cdot C(n-1, k-1)$（分子分母同乘法）  
5. $\sum_{i=0}^{k} C(n, i) = C(n+1, k+1)$（组合数求和）
6. $\sum_{k=0}^{n} C(n, k) * C(m, r-k) = C(n+m, r)$（组合数卷积）


???+ note "组合数计算方法"

    === "公式"

        ```cpp
        #include <cstdint>
        #include <vector>
        using namespace std;

        int64_t combination(int64_t n, int64_t m, int64_t mod = 1'000'000'007) {
          if (m > n || m < 0) { return 0; }
          m        = std::min(m, n - m);  // 利用对称性 C(n, m) == C(n, n-m)

          auto pow = [](int64_t a, int64_t b, int64_t mod) {
            int64_t res = 1 % mod;
            a           = a % mod;
            while (b > 0) {
              if ((b & 1) != 0) { res = res * a % mod; }
              a   = a * a % mod;
              b >>= 1;
            }
            return res;
          };

          // 以下计算过程中不取模, 可能会溢出, 适用于 n 比较小的情况
          // for (int64_t i = 1; i <= m; ++i) { res = res * (n - i + 1) / i; }

          // O(n) 预处理阶乘和逆元
          vector<int64_t> fact;          // 阶乘
          vector<int64_t> inverse_fact;  // 阶乘的逆元
          auto build = [&](int64_t n, int64_t mod) {
            fact.resize(n + 1, 1);
            inverse_fact.resize(n + 1, 1);
            for (int i = 2; i <= n; ++i) { fact[i] = fact[i - 1] * i % mod; }
            inverse_fact[n] = pow(fact[n], mod - 2, mod);
            for (int i = n; i > 1; --i) { inverse_fact[i - 1] = inverse_fact[i] * i % mod; }
          };
          build(n, mod);

          int64_t res = 1;
          // 计算C(n, m) = n!/(m!*(n-m)!)
          res = fact[n] * inverse_fact[m] % mod * inverse_fact[n - m] % mod;
          return res;
        }
        ```

    === "递推"

        ```cpp
        #include <cstdint>
        #include <vector>
        using namespace std;

        int64_t combination(int64_t n, int64_t m, int64_t mod = 1'000'000'007) {
          if (m > n || m < 0) { return 0; }
          m = std::min(m, n - m);  // 利用对称性 C(n, m) == C(n, n-m)

          vector<vector<int64_t>> binom(n + 1, vector<int64_t>(n + 1, 0));
          for (int i = 0; i <= n; ++i) {
            binom[i][0] = binom[i][i] = 1;
            for (int j = 1; j < i; ++j) { binom[i][j] = (binom[i - 1][j - 1] + binom[i - 1][j]) % mod; }
          }
          return binom[n][m];
        }
        ```

    === "因子计数法"

        ```cpp
        #include <cstdint>
        #include <vector>
        using namespace std;

        // 如果给定mod不是质数就用这种方法，或者考虑组合恒等式转化
        int64_t combination(int64_t n, int64_t m, int64_t mod = 1'000'000'007) {
          if (m > n || m < 0) { return 0; }
          m        = std::min(m, n - m);  // 利用对称性 C(n, m) == C(n, n-m)

          auto pow = [](int64_t a, int64_t b, int64_t mod) {
            int64_t res = 1 % mod;
            a           = a % mod;
            while (b > 0) {
              if ((b & 1) != 0) { res = res * a % mod; }
              a   = a * a % mod;
              b >>= 1;
            }
            return res;
          };

          vector<int64_t> minprime;  // 最小质因子
          vector<int64_t> primes;    // 质数表
          // 线性筛法求解最小质因子
          auto build_minprime = [&](int64_t n) {
            minprime.resize(n + 1, 0);
            for (int i = 2; i <= n; ++i) {
              if (minprime[i] == 0) {
                minprime[i] = i;
                primes.push_back(i);
              }
              for (int64_t p : primes) {
                if (p * i > n) { break; }
                minprime[p * i] = p;
                if (i % p == 0) { break; }
              }
            }
          };
          build_minprime(n);
          // 公式: C(n,m) = n! / (m! * (n-m)!)
          vector<int64_t> count(n + 1, 0);  // 质因子计数
          for (int i = 2; i <= m; ++i) {    // 分母部分 m!, 1不计入
            count[i] = -1;
          }
          for (int i = 2; i <= n - m; ++i) {  // 分母部分 (n-m)!
            count[i] = -1;
          }
          for (int i = 2; i <= n; ++i) {  // 分子部分 n!, 1不计入
            count[i] += 1;
          }
          for (int64_t i = n; i >= 2; --i) {
            if (count[i] == 0) { continue; }
            if (minprime[i] != i) {  // i 不是质数, 分解质因子
              int64_t p     = minprime[i];
              count[p]     += count[i];
              count[i / p] += count[i];
              count[i]      = 0;
            }
          }
          int64_t res = 1;
          for (int64_t i = 2; i <= n; ++i) {
            if (count[i] != 0) { res = res * pow(i, count[i], mod) % mod; }
          }
          return res % mod;
        }
        ```

## 二项式定理

二项式定理（$\text{Binomial Theorem}$）描述了二项式 $(x + y)^n$ 的展开形式。  
根据二项式定理，有：$(x + y)^n = \sum_{k=0}^{n} C(n, k) \cdot x^k \cdot y^{n-k}$。  

???+ node "二项式定理展开"
    返回 $(x + y)^n$ 展开后各项的系数。

    ```cpp
    #include <cstdint>
    #include <vector>
    using namespace std;

    int64_t inverse_euclid(int64_t a, int64_t mod) {
      using TIII        = tuple<int64_t, int64_t, int64_t>;
      auto extended_gcd = [](int64_t a, int64_t b) {
        auto y_combinator = [](auto &&self, int64_t a, int64_t b) -> TIII {
          if (b == 0) { return {a, 1, 0}; }
          auto [d, x1, y1] = self(self, b, a % b);
          int64_t x        = y1;
          int64_t y        = x1 - (a / b) * y1;
          return {d, x, y};
        };
        return y_combinator(y_combinator, a, b);
      };
      auto [d, x, y] = extended_gcd(a, mod);
      if (d != 1) { return -1; }  // a 和 mod 不互质, 无逆元
      return (x % mod + mod) % mod;
    }

    // 返回长度为 n+1 的数组, res[k] = C(n, k) * x^k * y^(n-k) for k=0,1,...,n
    vector<int64_t> binomial_expansion(int64_t n, int64_t x, int64_t y, int64_t mod = 1'000'000'007) {
      auto pow = [](int64_t a, int64_t b, int64_t mod) {
        int64_t res = 1 % mod;
        a           = a % mod;
        while (b > 0) {
          if ((b & 1) != 0) { res = res * a % mod; }
          a   = a * a % mod;
          b >>= 1;
        }
        return res;
      };

      vector<int64_t> res(n + 1, 0);
      int64_t x_pow = 1;               // x^k
      int64_t y_pow = pow(y, n, mod);  // y^(n-k), k=0时
      int64_t y_inv = inverse_euclid(y, mod);
      for (int64_t k = 0; k <= n; ++k) {
        res[k] = combination(n, k, mod) * x_pow % mod * y_pow % mod;
        x_pow  = x_pow * x % mod;
        y_pow  = y_pow * y_inv % mod;  // y^(n-k-1)
      }
      return res;
    }
    ```

??? node "[计算系数](https://www.luogu.com.cn/problem/P1313){target=_blank}"
    求解 $(ax + by)^{k}$ 展开后 $x^n y^m$ 的系数。

    ```cpp
    --8<-- "code/Math/Combination/P1313.cpp"
    ```

## 二项式反演

二项式反演（$\text{Binomial Inversion}$）是指通过已知序列 $f(n)$ 来求解另一个序列 $g(n)$，其中两者满足以下关系：  
记 $f_n$ 表示**恰好**使用 $n$ 个不同元素形成特定结构的方案数，$g_n$ 表示从 $n$ 个不同元素中选出 $i \geq 0$ 个元素形成特定结构的总方案数。
<br>

二项式反演的四种形式:  

1. **恰好**使用 $i$ 个元素的方案数 $f_i$ $\Longleftrightarrow$ $n$ 个元素中选出**任意**个元素的总方案数 $g_n$  

    $$
    g_n = \sum_{i=0}^{n} (-1)^{i} \binom{n}{i} f_i
    \quad \Longleftrightarrow \quad 
    f_n = \sum_{i=0}^{n} (-1)^{i} \binom{n}{i} g_i
    $$

    计数问题中，已知总方案数 $g_n$，需要反推出恰好使用 $n$ 个元素的方案数 $f_n$。

2. **恰好**使用 $i$ 个元素的方案数 $f_i$ $\Longleftrightarrow$ $n$ 个元素中选出**至少** $i$ 个元素的总方案数 $g_n$  

    $$
    g_n = \sum_{i=0}^{n} \binom{n}{i} f_i 
    \quad \Longleftrightarrow \quad 
    f_n = \sum_{i=0}^{n} (-1)^{(n-i)} \binom{n}{i} g_i
    \tag{1}
    $$

    钦定 $k$ 个且至少的问题，反推出恰好 $k$ 个的方案数。

3. **恰好**使用 $i$ 个元素的方案数 $f_i$ $\Longleftrightarrow$ 在给定上界 $N$ 的情况下，**至少**选 $n$ 个元素的总方案数 $g_n$  

    $$
    g_n = \sum_{i=n}^{N} (-1)^{i} \binom{i}{n} f_i 
    \quad \Longleftrightarrow \quad 
    f_n = \sum_{i=n}^{N} (-1)^{i} \binom{i}{n} g_i
    $$

    用于区间范围内的反演问题。

4. **恰好**使用 $i$ 个元素的方案数 $f_i$ $\Longleftrightarrow$ 在给定上界 $N$ 的情况下，**至多**选 $n$ 个元素的总方案数 $g_n$  

    $$
    g_n = \sum_{i=n}^{N} \binom{i}{n} f_i 
    \quad \Longleftrightarrow \quad 
    f_n = \sum_{i=n}^{N} (-1)^{(i-n)} \binom{i}{n} g_i
    $$

    钦定 $k$ 个且至多的问题，反推出恰好 $k$ 个的方案数。

形式 $2$ 和 $4$ 最常用。  
解决要求恰好 $k$ 个的问题, 可以转化为钦定 $k$ 个且至少的问题, 然后通过形式 $2$ 反演求解。  


??? node "[信封问题](https://www.luogu.com.cn/problem/P1595){target=_blank}"
    给定 $n$ 个信封和 $n$ 封信，问有多少种方法可以将信放入信封，使得没有任何一封信放入与其对应的信封中。

    === "递推公式"

        ??? hint

            考虑第 $1$ 个元素和另一个元素交换，另一个元素可以在剩下 $n-2$ 个元素中错排，或者和剩下 $n-1$ 个元素中的一个交换。第 $1$ 个元素可以和剩下 $n-1$ 个元素中的任意一个交换。  
            递推公式 $D(n) = (n-1) * (D(n-1) + D(n-2))$ 。

        ```cpp
        --8<-- "code/Math/Combination/P1595_1.cpp"
        ```

    === "二项式反演"

        ??? hint
        
            假设 $g_n$ 表示 $n$ 个元素的全排列数，$f_n$ 表示指定 $n$ 个元素的错排数，则有 $g_n = \sum_{i=0}^{n} C(n,i) * f_i$。反演得到 $f_n = \sum_{i=0}^{n} (-1)^{(n-i)} * C(n,i) * g_i$。  

        ```cpp
        --8<-- "code/Math/Combination/P1595_2.cpp"
        ```
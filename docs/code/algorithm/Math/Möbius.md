---
title: 莫比乌斯函数
tags:
  - 莫比乌斯函数
  - Möbius Function
  - 莫比乌斯反演
  - Möbius Inversion
  - 无平方因子数
  - Squarefree Number
  - 无平方因子核
  - Squarefree Kernel
---

# 莫比乌斯函数

莫比乌斯函数（$\text{Möbius function}$）是数论中一个重要的算术函数，记为 $\mu(n)$，定义如下：

$$
\mu(n) =
\begin{cases}
1, & n = 1, \\
(-1)^k, & n = p_1 p_2 \cdots p_k,\ \text{其中 } p_i \text{ 互不相同}, \\
0, & n \text{ 含有大于 $1$ 的平方因子}.
\end{cases}
$$

即：

- 当 $n = 1$ 时，$\mu(n) = 1$；
- 当 $n$ 是 无平方因子数（$\text{squarefree number}$），且有 $k$ 个不同质因子时，$\mu(n) = (-1)^k$；
- 当 $n$ 含有大于 $1$ 的平方因子时（即 $n$ 能被大于 $1$ 的平方数整除），$\mu(n) = 0$。

!!! abstract "无平方因子数"

    无平方因子数（$\text{Squarefree Number}$）是指不能被任何大于 $1$ 的平方数整除的正整数。换句话说，一个无平方因子数的质因子中，每个质因子的指数都是 $1$。

    !!! example 

        $6$ 是一个无平方因子数，因为它的质因子分解为 $2^1 \cdot 3^1$。
        
        $12$ 不是无平方因子数，因为它的质因子分解为 $2^2 \cdot 3^1$，其中 $2$ 的指数大于 $1$。

    形式上，如果一个正整数 $n$ 的质因子分解为：

    $$n = p_1^{e_1} \cdot p_2^{e_2} \cdots p_k^{e_k}$$

    其中 $p_i$ 是质数，$e_i$ 是对应的指数，那么当且仅当所有的 $e_i = 1$ 时，$n$ 是一个无平方因子数。

    使用数学符号表示为：$\mu (n)^{2} = 1$，其中 $\mu$ 是莫比乌斯函数。

    由于无平方因子数的所有素因数指数均为一次方，故除 $1$ 以外，有关数的正约数数目必定是 $2$ 的非负整数次方。

    !!! abstract "无平方因子数的正约数数目"
        
        设 $n$ 是一个无平方因子数，且其质因子分解为 $n = p_1^{1} \cdot p_2^{1} \cdots p_k^{1}$，则 $n$ 的正约数数目为：

        $$
        d(n) = \prod_{i=1}^{k} (\alpha_i + 1) = 2^k
        $$

        其中 $k$ 是 $n$ 的不同质因子的个数。

## 莫比乌斯函数的性质

1. 乘积性质

    对于任意正整数 $m$ 和 $n$，如果 $\gcd(m, n) = 1$，则有：

    $$
    \mu(mn) = \mu(m) \cdot \mu(n)
    $$

    莫比乌斯函数是积性函数。但是，当 $m$ 和 $n$ 不互质时，乘积性质不成立，说明莫比乌斯函数是非完全积性函数。

    ??? abstract "证明"

        设 $m$ 和 $n$ 的质因子分解分别为：

        $$
        m = p_1^{e_1} p_2^{e_2} \cdots p_k^{e_k}
        $$

        $$
        n = q_1^{f_1} q_2^{f_2} \cdots q_l^{f_l}
        $$

        由于 $\gcd(m, n) = 1$，所以 $m$ 和 $n$ 没有共同的质因子。根据莫比乌斯函数的定义，分析以下几种情况：

        - 如果 $m$ 或 $n$ 含有平方因子，则 $\mu(m) = 0$ 或 $\mu(n) = 0$，因此 $\mu(mn) = 0$，乘积性质成立。
        
        - 如果 $m$ 和 $n$ 都是无平方因子数，且分别有 $k$ 和 $l$ 个不同质因子，则 $mn$ 也是无平方因子数，且有 $k + l$ 个不同质因子。因此：

        $$
        \mu(m) = (-1)^k, \quad \mu(n) = (-1)^l, \quad \mu(mn) = (-1)^{k+l}
        $$

        因此，有：

        $$
        \mu(mn) = (-1)^{k+l} = (-1)^k \cdot (-1)^l = \mu(m) \cdot \mu(n)
        $$

2. 累加性质

    对于任意正整数 $n$，有：

    $$
    \sum_{d|n} \mu(d) =
    \begin{cases}
    1, & n = 1, \\
    0, & n > 1.
    \end{cases}
    $$

    其中，求和是对所有正约数 $d$ 进行的。

    ??? abstract "证明"

        设 $n$ 的质因子分解为 $n = p_1^{e_1} p_2^{e_2} \cdots p_k^{e_k}$。根据莫比乌斯函数的定义，只有当 $d$ 是无平方因子数时，$\mu(d)$ 才不为零。

        因此，只需考虑 $d$ 的形式为 $d = p_1^{f_1} p_2^{f_2} \cdots p_k^{f_k}$，其中每个 $f_i$ 要么是 $0$，要么是 $1$。这样，$d$ 的所有可能取值对应于 $n$ 的所有子集。

        对于每个子集，其对应的 $\mu(d)$ 值为 $(-1)^{\text{子集大小}}$。因此有：

        $$
        \sum_{d|n} \mu(d) = \sum_{j=0}^{k} \binom{k}{j} (-1)^j
        $$

        根据二项式定理，这个和等于 $(1 - 1)^k = 0^k$。当 $k > 0$ 时，结果为 $0$；当 $k = 0$（即 $n = 1$）时，结果为 $1$。

3. 无平方因子核

    定义 $\mathrm{rad}(n) = \prod_{p|n} p$，即 $n$ 的不同质因子之积。

    $$
    \mu(n) \ne 0 \iff n = \mathrm{rad}(n)
    $$

    ??? abstract "无平方因子核"

        无平方因子核（$\text{Squarefree Kernel}$，$\text{radical}$），也称为无平方因子部分，是指一个正整数中所有不同质因子的乘积。换句话说，无平方因子核是通过将一个数的质因子分解中每个质因子的指数都取为 $1$ 来得到的。

        形式上，如果一个正整数 $n$ 的质因子分解为：

        $$
        n = p_1^{e_1} \cdot p_2^{e_2} \cdots p_k^{e_k}
        $$

        其中 $p_i$ 是质数，$e_i$ 是对应的指数，那么无平方因子核可以表示为：

        $$
        \text{rad}(n) = p_1^{1} \cdot p_2^{1} \cdots p_k^{1}
        $$

        !!! example

            考虑正整数 $60$。它的质因子分解为：

            $$
            60 = 2^2 \cdot 3^1 \cdot 5^1
            $$

            因此，$60$ 的无平方因子核为：

            $$
            \text{rad}(60) = 2^1 \cdot 3^1 \cdot 5^1 = 30
            $$
    
    ???+ note "计算无平方因子核"

        === "单个数的无平方因子核计算"

            ```cpp
            int64_t rad(int64_t n) {
              int64_t res = 1;
              for (int64_t i = 2; i * i <= n; ++i) {
                if (n % i == 0) {
                  res *= i;
                  while (n % i == 0) {
                    n /= i;
                  }
                }
              }
              if (n > 1) { res *= n; }  // n 是质数
              return res;
            }
            ```

        === "线性筛法预处理无平方因子核"

            ```cpp
            auto rad_sieve(int64_t n) {
              static vector<int> core(n + 1);
              for (int i = 1; i <= n; ++i) {
                if (core[i] == 0) {  // i 不含完全平方因子，可以作为 core 值
                  for (int j = 1; i * j * j <= n; ++j) { core[i * j * j] = i; }
                }
              }
              return [&](int64_t x) { return core[x]; };
            }
            ```
    
    ??? note "[完全子集的最大元素和](https://leetcode.cn/problems/maximum-element-sum-of-a-complete-subset-of-indices/description/){target='_blank'}"

        给你一个下标从 $1$ 开始、由 $n$ 个整数组成的数组。你需要从 $nums$ 选择一个 完全集，其中每对元素下标的乘积都是一个完全平方数，例如选择 $a_i$ 和 $a_j$ ，$i * j$ 一定是完全平方数。

        返回完全子集所能取到的 最大元素和 。

        ```cpp
        --8<-- "code/Math/Möbius/maximum-element-sum-of-a-complete-subset-of-indices.cpp"
        ```



4. 与欧拉函数的关系

    莫比乌斯函数与欧拉函数 $\varphi(n)$ 之间存在以下关系：

    $$
    \varphi(n) = n \sum_{d|n} \frac{\mu(d)}{d}
    $$

    ??? abstract "证明"

        设 $n$ 的质因子分解为 $n = p_1^{e_1} p_2^{e_2} \cdots p_k^{e_k}$。根据欧拉函数的定义，有：

        $$
        \varphi(n) = n \prod_{i=1}^{k} \left(1 - \frac{1}{p_i}\right)
        $$

        可以将右侧展开为：

        $$
        \varphi(n) = n \left(1 - \sum_{i=1}^{k} \frac{1}{p_i} + \sum_{1 \leq i < j \leq k} \frac{1}{p_i p_j} - \cdots + (-1)^k \frac{1}{p_1 p_2 \cdots p_k}\right)
        $$

        注意到每一项都对应于某个无平方因子数 $d$，且 $\mu(d)$ 的值正好是该项的符号。因此，可以将上述表达式重新写为：

        $$
        \varphi(n) = n \sum_{d|n} \frac{\mu(d)}{d}
        $$

5. 平方和性质

    对于任意正整数 $n$，有：

    $$
    \sum_{d^2|n} \mu(d) =
    \begin{cases}
    1, & n \text{ 是无平方因子数}, \\
    0, & \text{否则}.
    \end{cases}
    $$

    ??? abstract "证明"

        设 $n$ 的质因子分解为 $n = p_1^{e_1} p_2^{e_2} \cdots p_k^{e_k}$。考虑所有满足 $d^2 | n$ 的正整数 $d$。

        由于 $d^2 | n$，所以 $d$ 的质因子分解中的每个质因子的指数必须不超过对应质因子在 $n$ 中指数的一半。换句话说，设 $d$ 的质因子分解为 $d = p_1^{f_1} p_2^{f_2} \cdots p_k^{f_k}$，则有 $2f_i \leq e_i$ 对所有 $i$ 成立。

        根据莫比乌斯函数的定义，只有当每个 $f_i$ 要么是 $0$，要么是 $1$ 时，$\mu(d)$ 才不为零。因此，只需考虑那些满足 $f_i \in \{0, 1\}$ 且 $2f_i \leq e_i$ 的情况。

        - 如果 $n$ 是无平方因子数，则所有的 $e_i = 1$。因此，只有当所有的 $f_i = 0$ 时，才能满足 $2f_i \leq e_i$。这时，唯一的符合条件的 $d$ 是 $1$，且 $\mu(1) = 1$。因此，有：

        $$
        \sum_{d^2|n} \mu(d) = \mu(1) = 1
        $$

        - 如果 $n$ 含有平方因子，则存在某个质因子 $p_j$ 使得 $e_j \geq 2$。在这种情况下，既可以选择 $f_j = 0$，也可以选择 $f_j = 1$，从而产生多个符合条件的 $d$。这些不同的选择会导致 $\mu(d)$ 的符号相互抵消，从而使得总和为零。因此，有：

        $$
        \sum_{d^2|n} \mu(d) = 0
        $$

## 计算莫比乌斯函数

可以在 $O(\sqrt{n})$ 时间内计算单个数的莫比乌斯函数值，或者使用线性筛法在 $O(n)$ 时间内预处理 $1$ 到 $n$ 范围内所有数的莫比乌斯函数值。

??? note "单个数的莫比乌斯函数计算"

    ```cpp
    int64_t mu(int64_t n) {
      if (n == 1) { return 1; }
      int64_t res = 1;
      for (int64_t i = 2; i * i <= n; ++i) {
        if (n % i == 0) {
          n /= i;
          if (n % i == 0) { return 0; }  // 含有平方因子
          res = -res;
        }
      }
      if (n > 1) { res = -res; }  // n 是质数
      return res;
    }
    ```

???+ note "线性筛法预处理莫比乌斯函数"

    ```cpp
    auto mu_sieve(int64_t n) {
      static vector<int64_t> mu(n + 1);
      vector<int64_t> primes;
      vector<bool> not_prime(n + 1);
      primes.reserve(n);
      mu[1] = 1;  // 定义 μ(1) = 1
      for (int64_t x = 2; x <= n; ++x) {
        if (!not_prime[x]) {
          primes.push_back(x);
          mu[x] = -1;  // 质数的莫比乌斯函数值为 -1
        }
        for (int64_t p : primes) {
          if (x * p > n) { break; }
          not_prime[x * p] = true;
          if (x % p == 0) {  // p 是 x 的一个质因子, 则 p^2 也是 x * p 的一个质因子
            mu[x * p] = 0;  // 含有平方因子的数的莫比乌斯函数值为 0
            break;
          }
          mu[x * p] = -mu[x];  // 互质数的乘积的莫比乌斯函数值为两个数值的乘积
        }
      }
      return [&](int64_t x) { return mu[x]; };
    }
    ```

## 莫比乌斯反演公式


如果 $f(n)$ 和 $g(n)$ 是定义在正整数上的两个算术函数，且满足：

$$
g(n) = \sum_{d|n} f(d)
$$

则有莫比乌斯反演公式：

$$
f(n) = \sum_{d|n} \mu(d) \cdot g\left(\frac{n}{d}\right)
$$

??? abstract "证明"

    根据定义，有：

    $$
    g(n) = \sum_{d|n} f(d)
    $$

    将 $g\left(\frac{n}{d}\right)$ 展开：

    $$
    g\left(\frac{n}{d}\right) = \sum_{k|\frac{n}{d}} f(k)
    $$

    因此，有：

    $$
    \sum_{d|n} \mu(d) \cdot g\left(\frac{n}{d}\right) = \sum_{d|n} \mu(d) \sum_{k|\frac{n}{d}} f(k)
    $$

    交换求和顺序，得到：

    $$
    \sum_{d|n} \mu(d) \cdot g\left(\frac{n}{d}\right) = \sum_{m|n} f(m) \sum_{d|\frac{n}{m}} \mu(d)
    $$

    根据莫比乌斯函数的累加性质，$\sum_{d|\frac{n}{m}} \mu(d)$ 等于 $1$ 当且仅当 $\frac{n}{m} = 1$，否则为 $0$。因此，只有当 $m = n$ 时，内层求和才不为零。

    最终得到：

    $$
    \sum_{d|n} \mu(d) \cdot g\left(\frac{n}{d}\right) = f(n)
    $$

??? note "[无平方因子的数](https://www.luogu.com.cn/problem/T134829){target=_blank}"

    给定区间 $[n, m]$，计数区间内的无平方因子数的个数。

    ```cpp
    --8<-- "code/Math/Möbius/T134829.cpp"
    ```
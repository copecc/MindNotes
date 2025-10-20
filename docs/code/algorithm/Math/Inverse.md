---
title: 逆元
tags:
  - 逆元
  - Inverse
  - 模逆元
  - Modular Inverse
---

# 逆元

在数学中，逆元（$\text{Inverse}$）是指在某种运算下与给定元素结合后能得到单位元的元素。  
对于加法运算，元素 $a$ 的逆元是 $-a$， 因为 $a + (-a) = 0$。  
对于乘法运算，元素 $a(a \neq 0)$ 的逆元是 $\frac{1}{a}$，因为 $a \times \frac{1}{a} = 1$。  
在模运算中，元素 $a$ 的模逆元是满足 $a \times a^{-1} \equiv 1 \mod m$ 的整数 $a^{-1}$，前提是 $a$ 与模数 $m$ 互质（$\gcd(a, m) = 1$）。  
此处只讨论模逆元。

## 费马小定理

!!! abstract "费马小定理"
    如果 $p$ 是**质数**，且 $a$ 是整数且不被 $p$ 整除（即 $\gcd(a, p) = 1$），则有：$a^{p-1} \equiv 1 \mod p$。于是：$a \times a^{p-2} \equiv a^{p-1} \equiv 1 \mod p$。  
    因此，当模数是质数时，可以通过计算 $a^{p-2} \bmod p$ 来得到 $a$ 的模逆元。

通过快速幂算法可以在 $O(\log p)$ 的时间内计算一个数的模逆元。  
通过逆元也可以计算除法：$\frac{a}{b} \equiv a \times b^{p-2} \mod p$。

???+ note annotate "费马小定理计算模逆元"

    ```cpp
    int64_t inverse(int x, int64_t mod = 1'000'000'007) {
      auto pow = [](int64_t x, int64_t n, int64_t mod = 1'000'000'007) {
        int64_t res  = 1;
        int64_t base = x % mod;
        while (n > 0) {
          // 如果 n 是奇数, 则需要将当前的 x 乘到结果上
          if ((n & 1) != 0) { res = (res * base) % mod; }
          base   = (base * base) % mod;
          n    >>= 1;
        }
        return res;
      };
      return pow(x, mod - 2, mod);
    }

    // 计算 a / b mod p (1)
    int64_t div(int64_t a, int b, int64_t mod = 1'000'000'007) {
      return (a % mod) * inverse(b, mod) % mod;
    }
    ```

1. 如果使用费马小定理计算 $\frac{a}{b} \bmod p$，需要满足以下条件：
      1. $p$ 是质数
      2. $b$ 和 $p$ 互质
      3. $\frac{a}{b}$ 是整数, 即 $a$ 能被 $b$ 整除

## 扩展欧几里得算法

费马小定理求解逆元只适用于模数为质数的情况。为了计算任意模数下的模逆元，可以使用扩展欧几里得算法。  
扩展欧几里得算法通过求解线性同余方程 $ax + by = 1$ 来找到 $x$，其中 $x$ 即为 $a$ 在模 $b$ 下的模逆元。  
该方法可以求出任意与 $b$ 互质的 $a$ 的模逆元。

!!! abstract "扩展欧几里得算法"
    给定两个整数 $a$ 和 $b$，扩展欧几里得算法用于求解线性同余方程 $ax + by = \gcd(a, b)$ 的整数解 $(x, y)$。  
    当 $a$ 和 $b$ 互质时（即 $\gcd(a, b) = 1$），该方程有整数解 $(x, y)$，其中 $x$ 即为 $a$ 在模 $b$ 下的模逆元。  
    通过递归地应用欧几里得算法，可以找到 $x$ 和 $y$ 的值。

???+ note "扩展欧几里得算法计算模逆元"

    ```cpp
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
    ```

## 线性时间预处理逆元

当需要计算 $n$ 个数的模逆元时，如果对每个数都使用费马小定理计算逆元，时间复杂度为 $O(n\log p)$。因此需要线性时间算法预处理来提高效率。  

!!! abstract annotate "递推公式"
    给定一个质数 $p$，计算 $1, 2, \ldots, n$ 的模逆元。  
    $\forall i \in [1, n], p = \lfloor \frac{p}{i} \rfloor * i + (p \bmod i)$(1)  
    $\Rightarrow \lfloor \frac{p}{i} \rfloor * i + (p \bmod i) \equiv 0 \mod p$  
    $\Rightarrow \lfloor \frac{p}{i} \rfloor * (p \bmod i)^{-1} + i^{-1} \equiv 0 \mod p$(2)  
    于是：$i^{-1} \equiv -\lfloor \frac{p}{i} \rfloor * (p \bmod i)^{-1} \mod p$。  
    <br>
    设 $i$ 的模逆元为 $inverse[i]$，则有以下递推公式：  
    $\begin{aligned}
    inverse[1] &= 1 \\
    inverse[i] &= (p - \lfloor \frac{p}{i} \rfloor) \times inverse[p \bmod i] \bmod p \quad (i > 1)
    \end{aligned}$

1. 整除与取余定义
2. 乘以 $i^{-1} * (p \bmod i)^{-1}$，前提是 $i$ 和 $p \bmod i$ 都与 $p$ 互质

???+ note "线性时间预处理逆元"

    ```cpp
    #include <vector>
    using namespace std;

    auto linear_inverse(int n, int64_t mod = 1'000'000'007) {
      static vector<int64_t> inverse({0, 1});  // 乘法逆元
      int size = inverse.size();
      if (inverse.size() <= n) { inverse.resize(n + 1, 1); }
      for (int i = size; i <= n; i++) {
        inverse[i] = (mod - mod / i) * inverse[mod % i] % mod;
      }
      return [&](int x) { return inverse[x]; };
    }
    ```

在计算组合数相关时，常常用到阶乘和阶乘逆元，这也可以使用线性时间预处理来计算。计算过程中也可以求出乘法逆元。

!!! abstract "阶乘和阶乘逆元"
    给定一个质数 $p$，计算 $1, 2, \ldots, n$ 的阶乘和阶乘逆元，以及对应的乘法逆元。  
    通过费马小定理可以计算出 $n!$ 的逆元为：$(n!)^{p-2} \bmod p$。  
    由 $((i-1)!)^{-1} \equiv i \cdot (i!)^{-1} \mod p$，可以递推计算出 $n-1, n -2, \dots, 1$ 的阶乘逆元。  
    由 $i^{-1} \equiv (i!)^{-1} \cdot (i-1)! \mod p$，可以递推计算出 $1, 2, \ldots, n$ 的乘法逆元。

???+ note "线性时间预处理阶乘和阶乘逆元"

    ```cpp
    #include <vector>
    using namespace std;

    auto factorial_inverse(int n, int64_t mod = 1'000'000'007) {
      auto pow = [](int64_t x, int64_t n, int64_t mod = 1'000'000'007) {
        int64_t res  = 1;
        int64_t base = x % mod;
        while (n > 0) {
          // 如果 n 是奇数, 则需要将当前的 x 乘到结果上
          if ((n & 1) != 0) { res = (res * base) % mod; }
          base   = (base * base) % mod;
          n    >>= 1;
        }
        return res;
      };

      static vector<int64_t> fac({1, 1});          // 阶乘
      static vector<int64_t> inverse_fac({1, 1});  // 阶乘的逆元
      static vector<int64_t> inverse({0, 1});      // 乘法逆元

      int size = fac.size();
      if (fac.size() <= n) {
        fac.resize(n + 1, 1);
        inverse_fac.resize(n + 1, 1);
        inverse.resize(n + 1, 1);
      }
      
      // O(n)预处理阶乘和逆元
      // 计算阶乘
      for (int i = size; i <= n; ++i) { fac[i] = fac[i - 1] * i % mod; }
      // 计算阶乘的逆元
      inverse_fac[n] = pow(fac[n], mod - 2, mod);
      for (int i = n; i > size; --i) { inverse_fac[i - 1] = inverse_fac[i] * i % mod; }
      // 计算乘法逆元
      for (int i = size; i <= n; ++i) { inverse[i] = inverse_fac[i] * fac[i - 1] % mod; }

      return [&](int64_t x) { return inverse[x]; };
    }
    ```






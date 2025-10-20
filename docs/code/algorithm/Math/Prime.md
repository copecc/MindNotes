---
title: 质数
tags:
  - 质数
  - Prime
  - 埃拉托斯特尼筛法
  - Sieve of Eratosthenes
  - 欧拉筛法
  - Sieve of Euler
  - 质因子
  - Prime Factorization
  - 欧拉函数
  - Euler's Totient Function
---

# 质数与筛法

质数（$\text{Prime}$）是指大于 $1$ 的自然数中，除了 $1$ 和它本身外，不能被其他自然数整除的数。  
任何大于 $1$ 的自然数都可以唯一地表示为质数的乘积，这被称为算术基本定理。  
为了有效地找出质数，可以使用筛法，常见的筛法有埃拉托斯特尼筛法和欧拉筛法。  

## 埃拉托斯特尼筛法

埃拉托斯特尼筛法（$\text{Sieve of Eratosthenes}$）的基本思想是：从 $2$ 开始，依次将每个质数的倍数标记为合数，直到处理完所有小于等于 $\sqrt{n}$ 的数。  
使用埃氏筛还能够高效地统计 $n$ 范围内的质数个数、计算不同质因子个数（$\omega$ 函数）以及所有质因子个数（$\Omega$ 函数(1)）等。  
时间复杂度为 $O(n \log \log n)$，适用于需要一次性找出所有质数的场景。
{.annotate}

1.  令 $p$ 为 $n$ 的最小质因子，$n$ 可以表示为 $n = p \times m$，于是 $m = n / p$。  
    去掉一个质因子 $p$ 后，关于"含重数的质因子个数"记作 $\Omega(n)$，有递推关系：$\Omega(n) = \Omega(m) + 1$。  
    递推的边界条件为 $\Omega(1) = 0$，因为 $1$ 没有质因子。

???+ note "埃拉托斯特尼筛法"
    === "埃拉托斯特尼筛法"

        ```cpp
        #include <cstdint>
        #include <vector>
        using namespace std;

        auto is_prime_Eratosthenes(int64_t n = 15'000'000) {
          static vector<bool> is_prime(n + 1, true);
          is_prime[0] = is_prime[1] = false;
          for (int64_t i = 2; i * i <= n; ++i) {
            if (is_prime[i]) {
              for (int64_t j = i * i; j <= n; j += i) { is_prime[j] = false; }
            }
          }
          return [&](int64_t x) { return is_prime[x]; };
        }
        ```
    
    === "计算质数个数"

        ```cpp
        #include <cstdint>
        #include <vector>
        using namespace std;

        int64_t count_prime_number(int64_t n) {
          if (n <= 1) { return 0; }
          vector<bool> is_prime(n + 1, true);
          int64_t count = (n + 1) / 2;  // 奇数个数加上`2`这个数
          for (int i = 3; i * i <= n; i += 2) {
            if (!is_prime[i]) { continue; }            // not a prime num
            for (int j = i * i; j <= n; j += i * 2) {  // 排除偶数
              if (is_prime[j]) {
                is_prime[j] = false;
                count--;
              }
            }
          }
          return count;
        }
        ```

    === "不同质因子个数"

        ```cpp
        #include <cstdint>
        #include <vector>
        using namespace std;

        auto omega(int64_t n = 15'000'000) {
          static vector<int64_t> count(n + 1, 0);
          for (int64_t i = 2; i <= n; ++i) {
            if (count[i] == 0) {  // a prime num
              for (int64_t j = i; j <= n; j += i) { count[j]++; }
            }
          }
          return [&](int64_t x) { return count[x]; };
        }
        ```

    === "所有质因子个数"

        ```cpp
        #include <cstdint>
        #include <vector>
        using namespace std;

        auto Omega(int64_t n = 15'000'000) {
          static vector<int64_t> count(n + 1, 0);
          vector<int64_t> minp(n + 1, 0);  // 最小质因子
          for (int64_t i = 2; i <= n; ++i) {
            if (minp[i] == 0) {  // i是素数
              for (int64_t j = i; j <= n; j += i) {
                if (minp[j] == 0) { minp[j] = i; }
              }
            }
          }
          // 计算每个数的所有素因子个数
          for (int64_t i = 2; i <= n; ++i) { count[i] = count[i / minp[i]] + 1; }
          return [&](int64_t x) { return count[x]; };
        }
        ```


!!! tip
    由于筛法需要从头开始预处理，所以不支持动态增大 $n$ 的范围

## 欧拉筛法

欧拉筛法（$\text{Sieve of Euler}$）的基本思想是：让每个合数只被它的最小质因子筛选一次，以达到不重复筛选的目的。  
欧拉筛法不仅可以找出质数，还可以用于计算每个数的最小质因子，从而实现快速的质因子分解。  
欧拉筛法的时间复杂度为 $O(n)$，适用于需要频繁判断质数的场景。  

???+ note "欧拉筛法"
    === "欧拉筛法"

        ```cpp
        #include <cstdint>
        #include <vector>
        using namespace std;

        auto is_prime_euler(int n = 15'000'000) {
          static vector<bool> is_prime(n + 1, true);
          is_prime[0] = is_prime[1] = false;
          vector<int> primes;
          primes.reserve((n / 2) + 1);
          for (int i = 2; i <= n; ++i) {
            if (is_prime[i]) { primes.push_back(i); }
            for (int p : primes) {
              if (i * p > n) { break; }
              is_prime[i * p] = false;
              if (i % p == 0) { break; }  // i has p as a prime factor
            }
          }
          return [&](int x) { return is_prime[x]; };
        }
        ```

    === "最小质因子"

        ```cpp
        #include <cstdint>
        #include <vector>
        using namespace std;

        vector<int64_t> min_prime_euler(int64_t n) {
          vector<int64_t> minp(n + 1, 0);  // minp[i] 表示 i 的最小质因子
          vector<int64_t> primes;
          for (int64_t i = 2; i <= n; ++i) {
            if (minp[i] == 0) {
              minp[i] = i;
              primes.push_back(i);
            }
            for (int64_t p : primes) {
              if (p > minp[i] || i * p > n) { break; }
              minp[i * p] = p;
            }
          }
          // 返回最小质因子数组, 如果是素数则 minp[i] = i
          return minp;
        }
        ```

## 欧拉函数

欧拉筛法还可以用来计算欧拉函数 $\varphi(n)$，即小于 $n$ 且与 $n$ 互质的正整数的个数。

!!! abstract "欧拉函数"
    欧拉函数 $\varphi(n)$ 表示 $[1, n]$ 中与 $n$ 互质的正整数的个数，即 $\varphi(n) = \left|\{ k \mid 1 \leq k \leq n, \gcd(k, n) = 1 \}\right|$。  
    欧拉函数有如下性质：

      1. 如果 $n$ 是质数，则 $\varphi(n) = n - 1$。
      2. 如果 $n = p_1^{a_1} p_2^{a_2} \cdots p_k^{a_k}$（$n$ 的质因数分解），则 $\varphi(n) = n (1 - \frac{1}{p_1}) (1 - \frac{1}{p_2}) \cdots (1 - \frac{1}{p_k})$。

    欧拉定理：如果 $\gcd(a, n) = 1$，则 $a^{\varphi(n)} \equiv 1 \ (\text{mod} \ n)$。

???+ note "欧拉函数"
    === "欧拉函数"

        ```cpp
        int64_t phi(int64_t n) {
          int64_t result = n;
          for (int64_t i = 2; i * i <= n; i++) {
            if (n % i == 0) {                 // i 是 n 的一个质因子
              while (n % i == 0) { n /= i; }  // 去掉所有 i 因子, 保证每个质因子只处理一次
              result -= result / i;           // 应用公式 result *= (1 - 1/i)
            }
          }
          if (n > 1) { result -= result / n; }  // 还有一个大于 sqrt(n) 的质因子
          return result;
        }
        ```

    === "线性时间计算欧拉函数"

        ```cpp
        #include <cstdint>
        #include <vector>
        using namespace std;

        vector<int64_t> euler_phi_sieve(int64_t n) {
          vector<int64_t> phi(n + 1);
          vector<int64_t> primes;
          phi[0] = 0;
          phi[1] = 1;
          for (int64_t i = 2; i <= n; ++i) {
            if (phi[i] == 0) {  // i 是素数
              phi[i] = i - 1;
              primes.push_back(i);
            }
            for (int64_t p : primes) {
              if (i * p > n) { break; }
              if (i % p == 0) {
                phi[i * p] = phi[i] * p;
                break;
              }
              phi[i * p] = phi[i] * (p - 1);
            }
          }
          return phi;
        }
        ```
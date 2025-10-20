---
title: 扩展欧几里得算法
tags:
  - 最大公约数
  - GCD
  - Greatest Common Divisor
  - 裴蜀定理
  - Bézout's Identity
  - 扩展欧几里得算法
  - Extended Euclidean Algorithm
  - 二元一次方程
  - Linear Diophantine Equation
  - 中国剩余定理
  - Chinese Remainder Theorem
---

# 扩展欧几里得算法

## 最大公约数

最大公约数（$\text{Greatest Common Divisor}$，$\text{GCD}$）是指两个或多个整数的共同约数中最大的一个。  
使用欧几里德算法可以高效地计算两个整数的最大公约数。  

欧几里德算法（辗转相除法）的基本思想是基于以下性质：

$$
\gcd(a, b) = \gcd(b, a \mod b)
$$

???+ note "欧几里德算法"

    时间复杂度 $O(log(min(a, b)))$  

    ```cpp

    int64_t gcd(int64_t a, int64_t b) {
      while (b != 0) {
        int64_t temp = a % b;
        a            = b;
        b            = temp;
      }
      return a;
    }
    ```

!!! tip "更相减损术"

    更相减损术基于以下性质：

    $$
    \gcd(a, b) = \gcd(a, b - a) \quad \text{如果 } a < b
    $$

    该方法通过不断地用较大的数减去较小的数，直到两数相等，此时的数即为最大公约数。  
    这种方法在某些情况下可能比辗转相除法更直观，但在效率上通常不如辗转相除法。  

    ```cpp
    int64_t gcd_subtraction(int64_t a, int64_t b) {
      while (a != b) {
        if (a > b) {
          a -= b;
        } else {
          b -= a;
        }
      }
      return a;  // 或者 return b; 因为此时 a == b
    }
    ```

    推广到多个数的最大公约数，以下性质成立：

    $$
    \gcd(a_1, a_2, \ldots, a_n) = \gcd(a_1, a_2 - a_1, a_3 - a_2, \ldots, a_n - a_{n-1})
    $$

    因为：

    $$
    \gcd(a_1, a_2, \ldots, a_n) = \gcd(\gcd(a_1, a_2), \gcd(a_2, a_3), \ldots, \gcd(a_{n-1}, a_n))
    $$

    利用这个性质可以利用差分数组以及线段树维护区间的最大公约数。



## 裴蜀定理与扩展欧几里德算法

裴蜀定理（$\text{Bézout's Identity}$）指出，对于任意整数 $a$ 和 $b$，存在整数 $x$ 和 $y$ 使得：

$$
ax + by = \gcd(a, b)
$$

其中，$x$ 和 $y$ 被称为裴蜀系数（$\text{Bézout coefficients}$）。  
扩展欧几里德算法（$\text{Extended Euclidean Algorithm}$）不仅可以计算最大公约数，还可以找到裴蜀系数。

???+ abstract "裴蜀定理的推论"

    1. 如果 $a$ 和 $b$ 互质（即 $\gcd(a, b) = 1$），则存在整数 $x$ 和 $y$ 使得：  

        $$
        ax + by = 1
        $$  

        这意味着 $a$ 在模 $b$ 意义下有乘法逆元（$ax \equiv 1 \mod b$），反之亦然。  

    2. 对于任意整数 $c$，方程 $ax + by = c$ 有整数解的充分必要条件是 $\gcd(a, b)$ 整除 $c$。  
       如果 $(x_0, y_0)$ 是该方程的一个特解，则所有解可以表示为：

        $$
        x = x_0 + \frac{b}{d}t, \quad y = y_0 - \frac{a}{d}t
        $$

        其中，$d = \gcd(a, b)$，$t$ 是任意整数。  
     
     3. 如果 $a$ 和 $b$ 互质，则方程 $ax + by = c$ 对任意整数 $c$ 都有整数解。  
     4. 对于 $n$ 个整数 $a_1, a_2, \ldots, a_n$，存在整数 $x_1, x_2, \ldots, x_n$ 使得：

        $$
        a_1x_1 + a_2x_2 + \ldots + a_nx_n = \gcd(a_1, a_2, \ldots, a_n)
        $$

        这可以通过递归地应用扩展欧几里德算法来实现。  

扩展欧几里德算法可以计算整数 $a$ 和 $b$ 的最大公约数 $d = \gcd(a, b)$，以及满足 $ax + by = d$ 的整数解 $(x, y)$。

???+ note "扩展欧几里德算法"

    时间复杂度 $O(log(min(a, b)))$  

    ```cpp
    #include <tuple>
    using namespace std;

    auto extended_gcd = [](int64_t a, int64_t b) {
      using TIII        = tuple<int64_t, int64_t, int64_t>;
      auto y_combinator = [](auto &&self, int64_t a, int64_t b) -> TIII {
        if (b == 0) { return {a, 1, 0}; }
        auto [d, x1, y1] = self(self, b, a % b);
        int64_t x        = y1;
        int64_t y        = x1 - (a / b) * y1;
        return {d, x, y};
        // 最小非负整数解(1)
        // x = (x % (b / d) + (b / d)) % (b / d);
      };
      return y_combinator(y_combinator, a, b);
    };
    ```

    1. 最小非负整数解：所有解都在模 $m = \frac{b}{d}$ 意义下等价。要取最小非负整数解（即该等价类在区间 $[0, m-1]$ 的代表），只需把某个解 $x$ 取模 $m$ 得到规范代表。

## 二元一次方程

二元一次方程（丢番图方程，$\text{Linear Diophantine Equation}$）是指形如 $ax + by = c$ 的方程，其中 $a$、$b$ 和 $c$ 是已知整数，$x$ 和 $y$ 是未知整数。  
使用扩展欧几里德算法可以找到该方程的一组整数解（如果存在）。  

??? note "[【模板】二元一次不定方程 (exgcd)](https://www.luogu.com.cn/problem/P5656){target=_blank}"

    给定 $a, b, c$，讨论 $ax + by = c$ 的整数解的情况，并输出相关信息。  
    如果无解，输出 $-1$。如果有整数解但是无正整数解，输出所有解中 $x$ 的最小正整数解和 $y$ 的最小正整数解。如果有正整数解，输出正整数解的个数，以及 $x$ 和 $y$ 的最小正整数解和最大正整数解。  

    ```cpp
    --8<-- "code/Math/GCD/P5656.cpp"
    ```

## Frobenius数

对于两个互质的正整数 $a$ 和 $b$，$\text{Frobenius}$ 数是指无法表示为 $ax + by$ 的最大整数，其中 $x$ 和 $y$ 是非负整数。  
$\text{Frobenius}$ 数可以通过以下公式计算：  

$$
g(a, b) = ab - a - b
$$

??? question "为什么是 $ab - a - b$ ？"
    
    设 $N = ab - a - b$，假设存在非负整数 $x$ 和 $y$ 使得 $N = ax + by$。  
    则有：

    $$
    ab - a - b = ax + by
    $$

    将等式两边同时加上 $a + b$，得到：

    $$
    ab = a(x + 1) + b(y + 1)
    $$

    由于 $a$ 和 $b$ 互质，$a$ 必须整除 $y + 1$，即存在整数 $k$ 使得 $y + 1 = ak$。  
    同理，$b$ 必须整除 $x + 1$，即存在整数 $m$ 使得 $x + 1 = bm$。  
    将这些表达式代入原始方程，得到：

    $$
    ab = a(bm) + b(ak) \Leftrightarrow
    ab = abm + abk \Leftrightarrow
    1 = m + k
    $$

    因为 $m$ 和 $k$ 都是非负整数，所以唯一的解是 $m = 1$ 和 $k = 0$ 或者 $m = 0$ 和 $k = 1$。  
    对应的 $(x, y)$ 分别为 $(b - 1, 0)$ 和 $(0, a - 1)$。  
    然而，这两个解都不满足原始方程中的非负整数条件，因为它们会导致其中一个变量为负数。  
    因此，假设不成立，即不存在非负整数 $x$ 和 $y$ 使得 $N = ax + by$。  
    <br>
    接下来，需要证明对于任何大于 $N$ 的整数，都存在非负整数 $x'$ 和 $y'$ 使得该整数可以表示为 $ax' + by'$。  
    对于任意整数 $M > N$，可以表示为：

    $$
    M = N + k
    $$

    其中，$k$ 是一个正整数。  
    可以选择适当的非负整数 $x'$ 和 $y'$ 来满足方程，如果选择 $(x', y') = (x + k, y)$，则有：

    $$
    M = a(x + k) + by = ax + by + ak = N + ak
    $$

    这表明 $M$ 可以表示为 $ax' + by'$，其中 $x' = x + k$ 和 $y' = y$。  
    因此，对于任何大于 $N$ 的整数，都可以找到非负整数解。

## 中国剩余定理

中国剩余定理（$\text{Chinese Remainder Theorem}$，$\text{CRT}$）用于求解一组同余方程。  
假设有 $k$ 个整数 $m_1, m_2, \ldots, m_k$，它们两两互质，以及 $k$ 个整数 $r_1, r_2, \ldots, r_k$，则存在一个整数 $x$，使得：  

$$
\begin{aligned}
x & \equiv r_1 \mod m_1 \\
x & \equiv r_2 \mod m_2 \\
  & \vdots \\
x & \equiv r_k \mod m_k
\end{aligned}
$$

并且这个解在模 $M = m_1 m_2 \cdots m_k$ 意义下是唯一的。  
使用扩展欧几里得算法可以求解该方程组的一组解, 返回最小非负整数解 $x$ 和模数 $M$。  

??? question "为什么可以使用扩展欧几里得算法求解中国剩余定理？"

    设 $M = m_1 m_2 \cdots m_k$，对于每个 $i$，定义 $M_i = \frac{M}{m_i}$。  
    由于 $m_i$ 和 $M_i$ 互质，根据裴蜀定理，存在整数 $y_i$ 和 $z_i$ 使得：

    $$
    M_i y_i + m_i z_i = 1
    $$

    这意味着 $M_i y_i \equiv 1 \pmod{m_i}$。  
    因此，我们可以构造解 $x$ 如下：

    $$
    x = \sum_{i=1}^{k} r_i M_i y_i
    $$

    这个解满足所有的同余条件，因为对于每个 $i$：

    $$
    x \equiv r_i M_i y_i \equiv r_i \cdot 1 \equiv r_i \pmod{m_i}
    $$

    因为其他项都包含因子 $m_j$（$j \neq i$），所以在模 $m_i$ 意义下它们都为零。  
    最终，解 $x$ 在模 $M$ 意义下是唯一的，因为如果存在另一个解 $x'$，则有：

    $$
    x \equiv x' \pmod{m_i}
    $$

    对所有的 $i$ 成立，因此 $x - x'$ 是所有 $m_i$ 的公倍数，即：  

    $$
    x - x' \equiv 0 \pmod{M}
    $$

    这证明了解的唯一性。

??? note "[【模板】中国剩余定理（CRT）/ 曹冲养猪](https://www.luogu.com.cn/problem/P1495){target=_blank}"

    给定 $n$ 个两两互质的正整数 $m_i$ 和对应的余数 $r_i$，求解同余方程组，并输出最小非负整数解。

    ```cpp
    --8<-- "code/Math/GCD/P1495.cpp"
    ```

### 扩展中国剩余定理

当模数不互质时，仍然可以使用扩展中国剩余定理来求解同余方程组。此时该方程组有解的充分必要条件是对于 $\forall i, j:\; (r_i - r_j) \bmod \gcd(m_i, m_j) = 0$。


??? note "[【模板】扩展中国剩余定理（EXCRT）](https://www.luogu.com.cn/problem/P4777){target=_blank}"

    给定 $n$ 个正整数 $m_i$（不一定互质）和对应的余数 $r_i$，求解同余方程组，并输出最小非负整数解。如果无解，输出 $-1$。

    ```cpp
    --8<-- "code/Math/GCD/P4777.cpp"
    ```
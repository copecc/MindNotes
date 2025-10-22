---
title: 快速幂
tags:
  - 快速幂
  - Quick Power
  - 矩阵快速幂
  - Matrix Quick Power
---

# 快速幂

## 乘法快速幂

乘法快速幂（$\text{Quick Power}$）是一种高效计算大整数幂 $x^n, n \geq 0$ 的算法，时间复杂度为 $O(\log n)$。
基本思想是将指数 $n$ 分解为二进制形式：$n = b_k*2^k + ... + b_0*2^0, b_i \in \{0, 1\}$。因此：$x^n = x^{b_k*2^k} * ... * x^{b_0*2^0}$。  
不断平方底数，指数右移一位，遇到 $b_i=1$ 就乘到结果上。  
同理，整数乘法也可以用类似的方法实现。  


???+ note "乘法快速幂"
    === "快速幂"

        ```cpp
        int64_t pow(int64_t x, int64_t n, int64_t mod = 1'000'000'007) {
          int64_t res  = 1;
          int64_t base = x % mod;
          while (n > 0) {
            if ((n & 1) != 0) { res = (res * base) % mod; }
            base   = (base * base) % mod;
            n    >>= 1;
          }
          return res;
        }
        ```

    === "整数乘法"

        ```cpp
        int64_t multiply(int64_t a, int64_t b, int64_t mod = 1'000'000'007) {
          int64_t result = 0;
          a              = (a % mod + mod) % mod;
          b              = (b % mod + mod) % mod;
          while (b > 0) {
            if ((b & 1) != 0) { result = (result + a) % mod; }
            a   = (a * 2) % mod;
            b >>= 1;
          }
          return result % mod;
        }
        ```

!!! tip 
    乘法快速幂的应用

    - 计算大整数的幂模
    - 计算组合数
    - 计算逆元

## 矩阵快速幂

矩阵快速幂（$\text{Matrix Quick Power}$）是一种高效计算矩阵的整数次幂 $A^n, n \geq 0$ 的算法，时间复杂度为 $O(k^3 \log n)$，其中$A$为$k \times k$的方阵。  
基本思想与乘法快速幂类似，将指数 $n$ 分解为二进制形式，并通过不断平方矩阵来计算幂。  

???+ note "[【模板】矩阵快速幂](https://www.luogu.com.cn/problem/P3390){target=_blank}"

    ```cpp
    --8<-- "code/Math/QuickPow/P3390.cpp"
    ```
    
??? note "矩阵乘法"

    对于两个矩阵 $A$ 和 $B$，它们的乘积 $C = A * B, A \in \mathbb{R}^{m \times n}, B \in \mathbb{R}^{n \times p}$，则 $C \in \mathbb{R}^{m \times p}$，其中 $C_{ij} = \sum_{k=1}^{n} A_{ik} * B_{kj}$。  

    ```cpp

    using VVI = vector<vector<int64_t>>;

    // 矩阵乘法，计算 a * b, 其中 a 是 m*n 矩阵, b 是 n*p 矩阵
    VVI multiply(const VVI &a, const VVI &b, int64_t mod = 1'000'000'007) {
      int m = a.size(), n = a[0].size(), p = b[0].size();
      int64_t zero = 0;  // 零元
      VVI c(m, vector<int64_t>(p, zero));

      for (int i = 0; i < m; i++) {
        for (int k = 0; k < n; k++) {
          for (int j = 0; j < p; j++) { c[i][j] = (c[i][j] + a[i][k] * b[k][j] % mod) % mod; }
        }
      }
      return c;
    }
    ```

矩阵快速幂常用于递推问题的高效计算。  
对于一维 $k$ 阶线性递推关系：$F(n) = c_{1} \cdot F(n-1) + c_{2} \cdot F(n-2) + \dots + c_{k} \cdot F(n-k)$，如果使用行向量表示递推，可以构造转移矩阵 $M$ 满足： $[F(n), F(n-1), \dots, F(n-k+1)] = [F(n-1), F(n-2), \dots, F(n-k)] * M$，其中 $M$ 为：

$$
M = \begin{bmatrix}
c_1 & 1 & 0 & \cdots & 0 \\
c_2 & 0 & 1 & \cdots & 0 \\
\vdots & \vdots & \vdots & \ddots & \vdots \\
c_{k-1} & 0 & 0 & \cdots & 1 \\
c_k & 0 & 0 & \cdots & 0
\end{bmatrix}
$$

即：  

- 第一列是递推系数 $[c_1, c_2, \dots, c_k]$
- 其余是单位矩阵右移一列（即 $I_{k-1}$）  

由此可得：$[F(n), F(n-1), \dots, F(n-k+1)] = [F(k), F(k-1), \dots, F(1)] * M^{n-k}$，
因此，用矩阵快速幂即可高效求解 $F(n)$（注意初始行向量和矩阵乘法顺序）  

??? note "[斐波那契数](https://leetcode.cn/problems/fibonacci-number/description/){target=_blank}"

    !!! example "斐波那契数列"

        斐波那契数列满足递推关系 $F(n) = F(n-1) + F(n-2)$，初始条件为 $F(0) = 0, F(1) = 1$。  
        构造转移矩阵：

        $$
        M = \begin{bmatrix}
        1 & 1 \\
        1 & 0
        \end{bmatrix}
        $$

        则有：

        $$
        \begin{bmatrix}
        F(n) & F(n-1)
        \end{bmatrix} = \begin{bmatrix}
        F(1) & F(0)
        \end{bmatrix} * M^{n-1}
        $$

        因此，可以通过计算矩阵 $M^{n-1}$ 来高效求解 $F(n)$。
      


    ```cpp
    --8<-- "code/Math/QuickPow/L509.cpp"
    ```

## 广义矩阵快速幂

广义上，只要两个满足结合律的代数系统，都可以使用类似的方法进行快速幂计算。  
即满足：内层的运算具有结合律，外层的运算对于内层运算有分配律。(1)  
{.annotate}

1. 运算符 $A$ 对于运算符 $B$ 有分配律, 如果对于所有 $a, b, c \in S$ 都满足 $A(a, B(b, c)) = B(A(a, b), A(a, c))$ 和 $A(B(b, c), a) = B(A(b, a), A(c, a))$。

    !!! example "$*$ 对 $+$ 有分配律"
        对于所有 $a, b, c \in \mathbb{R}$ 都满足 $a * (b + c) = a * b + a * c$ 和 $(b + c) * a = b * a + c * a$。

!!! tip "半环上的矩阵快速幂"

    一般来说，如果加法和乘法满足半环（$semi-ring$）的公理，那么矩阵乘法的结合律成立，从而可以使用矩阵快速幂优化动态规划。[^1]  
    [^1]: [AtCoder ABC236 G - Good Vertices Editorial](https://atcoder.jp/contests/abc236/editorial/3305){target=_blank}

    半环（$semi-ring$）的定义：  
    半环是一个集合 $A$，配备了加法 $\oplus$ 和乘法 $\otimes$两个二元运算，并满足以下性质：  

    1. $(A, \oplus)$ 是一个交换幺半群（$\text{commutative monoid}$），即满足以下三个条件：

        1. 加法的结合律成立：对于任意 $a, b, c \in A$，有 $(a \oplus b) \oplus c = a \oplus (b \oplus c)$  
        2. 存在加法的单位元 $0$：存在 $0 \in A$，使得 $a \oplus 0 = 0 \oplus a = a$  
        3. 加法满足交换律：对于任意 $a, b \in A$，有 $a \oplus b = b \oplus a$  

    2. $(A, \otimes)$ 是一个幺半群（$\text{monoid}$），即满足以下两个条件：  

        1. 乘法的结合律成立：对于任意 $a, b, c \in A$，有 $(a \otimes b) \otimes c = a \otimes (b \otimes c)$  
        2. 存在乘法的单位元 $1$：存在 $1 \in A$，使得 $a \otimes 1 = 1 \otimes a = a$  
    
    3. 加法和乘法满足以下分配律：  

        1. 左分配律：对于任意 $a, b, c \in A$，有 $a \otimes (b \oplus c) = a \otimes b \oplus a \otimes c$  
        2. 右分配律：对于任意 $a, b, c \in A$，有 $(a \oplus b) \otimes c = a \otimes c \oplus b \otimes c$  
        3. 乘法对 $0$ 的吸收性：对于任意 $a \in A$，有 $0 \otimes a = a \otimes 0 = 0$  

    根据不同的加法和乘法定义，初始化矩阵时：  

    - 矩阵的所有元素初始化为加法 $\oplus$ 的零元$0$  
    - 单位矩阵的主对角线元素初始化为乘法 $\otimes$ 的单位元$1$  
    
    对于满足以上条件的代数系统, 使用对应的操作替代矩阵乘法中的加法和乘法即可。

    !!! example annotate "常见的半环"

        1. $A := \mathbb{R}$, $\oplus$ 是普通加法，$\otimes$ 是普通乘法（普通的矩阵乘法）(1)  
        2. $A := \mathbb{R}$，$\oplus$ 是 $min$，$\otimes$ 是 $max$(2)  
        3. $A := \mathbb{N}$，$\oplus$ 是 $gcd$，$\otimes$ 是 $lcm$(3)   
        4. $A := \mathbb{R} \cup \{-\infty\}$，$\oplus$ 是 $max$，$\otimes$ 是普通加法  
        5. 对于一个正整数 $n$，$A$ 是小于 $2^n$ 的非负整数集合，$\oplus$ 是按位逻辑或，$\otimes$ 是按位逻辑与  
        6. 对于一个正整数 $n$，$A$ 是小于 $2^n$ 的非负整数集合，$\oplus$ 是按位异或，$\otimes$ 是按位逻辑与  

        根据加法 $\oplus$ 和乘法 $\otimes$ 的定义，零元和单位元(4)的选择如下：  

        $$
        \begin{array}{c|c|c|c|c}
        \text{集合 }A & \text{加法 }\oplus & \text{零元 }0 & \text{乘法 }\otimes & \text{单位元 }1\\
        \hline
        \mathbb{R} & \text{ordinary }+ & 0 & \text{ordinary }* & 1\\
        \mathbb{R} & \min & +\infty & \max & -\infty\\
        \mathbb{R} & \max & -\infty & \min & +\infty\\
        \mathbb{N} & \gcd & 0 & \mathrm{lcm} & 1\\
        \mathbb{R}\cup\{-\infty\} & \max & -\infty & \text{ordinary }+ & 0\\
        \mathbb{R}\cup\{+\infty\} & \min & +\infty & \text{ordinary }+ & 0\\
        \forall n\in\mathbb{N}^+,\ \{0,\dots,2^n-1\} & \text{bitwise OR} & 0 & \text{bitwise AND} & 2^n-1\\
        \forall n\in\mathbb{N}^+,\ \{0,\dots,2^n-1\} & \text{bitwise AND} & 2^n-1 & \text{bitwise OR} & 0
        \end{array}
        $$
    
    1. $+$ 和 $*$ 满足结合律, 且 $*$ 对 $+$ 有分配律
    2. $min$ 和 $max$ 满足结合律, 且 $max$ 对 $min$ 有分配律
    3. $gcd$ 和 $lcm$ 满足结合律, 且 $lcm$ 对 $gcd$ 有分配律
    4.  !!! example annotate "零元和单位元"

            - 对于 $(+, *)$：$(A, +)$ 零元为 $0$，$(A, *)$ 单位元为 $1$ (1)  
            - 对于 $(min, max)$：$(A, min)$ 零元为 $+\infty$，$(A, max)$ 单位元为 $-\infty$ (2)

        1.  $a + 0 = a, a * 1 = a$
        2.  $min(a, +\infty) = a, max(a, -\infty) = a$
  
!!! example "$min-max$ 矩阵快速幂"
    设有两个矩阵 $A, B \in \mathbb{R}^{m \times n}$, 定义矩阵乘法为 $C = A * B, C \in \mathbb{R}^{m \times n}$, 其中 $c[i][j] = min\{max\left(a[i][k], b[k][j]\right), k \in [1, n]\}$。  
    则可以使用类似的矩阵快速幂方法计算 $C^n$。  
    代码实现与上述类似, 只需替换乘法和加法为对应的 $min$ 和 $max$ 操作即可。  
    该方法一般用于求解最小化最大边权问题（$\text{bottleneck / minimax}$）。


??? note "[Good Vertices](https://atcoder.jp/contests/abc236/tasks/abc236_g){target=_blank}"
    一个有向图，包含 $N$ 个顶点，编号为 $1, 2, \dots, N$。  
    初始时刻 $t = 0$，图中没有任何边。在每个时刻 $t = 1, 2, \dots, T$，添加一条有向边：从顶点 $u_t$ 到顶点 $v_t$（可能是自环，即 $u_t = v_t$）。  
    如果从顶点 $1$ 出发，经过恰好 $L$ 条边可以到达顶点 $i$，则称顶点 $i$ 是 **Good** 的。  
    对于每个顶点 $i = 1, 2, \dots, N$：输出使顶点 $i$ 成为 **Good** 的最早时刻 $t$。如果不存在这样的时刻，输出 $-1$。  

    ??? hint
    
        将时间 $t$ 看作边权，边权为 $t$ 的边表示在时刻 $t$ 添加的边。

        目标是找到从顶点 $1$ 出发，经过恰好 $L$ 边权到达顶点 $i$ 的路径中，最大边权的最小值。

    ```cpp
    --8<-- "code/Math/QuickPow/abc236_g.cpp"
    ```
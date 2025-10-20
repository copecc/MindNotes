---
title: 快速傅里叶变换
tags:
  - 快速傅里叶变换
  - FFT
  - 快速数论变换
  - NTT
---

# 快速傅里叶变换

## FFT

快速傅立叶变换（$\text{Fast Fourier Transform，FFT}$）是一种高效实现离散傅里叶变换（$\text{DFT}$）的算法。

$\text{FFT}$ 算法的基本思想是分治。就 $\text{DFT}$ 来说，它分治地来求当 $x = \omega_n^k$ 的时候 $f(x)$ 的值。

$\text{FFT}$ 算法的分治思想体现在将多项式分为奇次项和偶次项处理，分别用奇偶次次项数建立新的函数：$f(x) = Even(x^2) + x * Odd(x^2)$。

其中$Even(x) = \sum a_{2i}x^i$，$Odd(x) = \sum a_{2i+1}x^i$。

利用偶数次单位根的性质 $\omega^i_n = -\omega^{i + n/2}_n$，而$Even\left(x^2\right)$和$Odd\left(x^2\right)$是偶函数，因此在复平面上 $\omega^i_n$ 和 $\omega^{i+n/2}_n$ 对应的 $Even(x^2)$ 和 $Odd(x^2)$ 的值相同。以下推导中，令 $n$ 为多项式的次数，$k$ 为当前求值的下标。

$$
\begin{aligned}
f(\omega_n^k) &= Even((\omega_n^k)^2) + \omega_n^k  \times Odd((\omega_n^k)^2) \\
              &= Even(\omega_n^{2k}) + \omega_n^k  \times Odd(\omega_n^{2k}) \\
              &= Even(\omega_{n/2}^k) + \omega_n^k  \times Odd(\omega_{n/2}^k)
\\
f(\omega_n^{k+n/2}) &= Even(\omega_n^{2k+n}) + \omega_n^{k+n/2}  \times Odd(\omega_n^{2k+n}) \\
                    &= Even(\omega_n^{2k}) - \omega_n^k  \times Odd(\omega_n^{2k}) \\
                    &= Even(\omega_{n/2}^k) - \omega_n^k  \times Odd(\omega_{n/2}^k)
\end{aligned}
$$

因此求出了 $Even(\omega_{n/2}^k)$ 和 $Odd(\omega_{n/2}^k)$ 后，就可以同时求出 $f(\omega_n^k)$ 和 $f(\omega_n^{k+n/2})$。

分治 $\text{DFT}$ 能处理的多项式长度只能是 $2^m(m \in \mathbf{N}^ \ast )$，否则在分治的时候左右不一样长，右边取不到系数。所以要在第一次 $\text{DFT}$ 之前就把序列向上补成长度为 $2^m$（高次系数补 $0$）、最高项次数为 $2^m-1$ 的多项式。

在代入值的时候，因为要代入$n$个不同值：$\omega_n^0,\omega_n^1,\omega_n^2,\cdots, \omega_n^{n-1} (n=2^m)$ 一共 $2^m$ 个不同值。

注意：

1. 在调用 $\text{FFT}$ 前，需要将多项式的系数转化为复数，同时要将系数调整为 $2^m$ 长度的多项式。
2. 在求快速傅里叶变换的逆变换时，注意角度要乘以 $-1$，这在代码中通过 `inv` 参数来控制。
3. 求出快速傅里叶变换的逆变换后，需要正则化，将结果除以 $n$。
4. 多项式乘法$h(x) = f(x) \times g(x)$，可以通过 $h(x) = \text{FFT}^{-1}(\text{FFT}(f(x)) \times \text{FFT}(g(x)))$ 来实现。在最后的结果中，需要将系数四舍五入取整。

!!! tip "$w_k$ 的计算方式"

    $\text{FFT}$:

    $$
    w_k = e^{\frac{2\pi}{n} k i} = e^{\theta k i} = cos(\theta k) + i * sin(\theta k)
    $$

    $\text{Inverse FFT}$:

    $$
    w_k = e^{- \frac{2\pi}{n} k i} =e^{- \theta k i} = cos(\theta k) - i * sin(\theta k)
    $$

    递推关系：

    $$
    w_{k+1} = e^{\theta (k+1) * i} = e^{\theta k * i} * e^{\theta * i} = w(k) * w
    $$

???+ note "[【模板】多项式乘法（FFT）](https://www.luogu.com.cn/problem/P3803){target='_blank'}"

    给定两个多项式 $f(x)$ 和 $g(x)$，求它们的乘积多项式 $h(x) = f(x) \times g(x)$。

    !!! keypoints

        一般来说，$\text{FFT}$ 的实现分为两种：递归实现和优化实现。递归实现的代码较为简单，优化实现则在此基础上进行了位逆序置换的优化。


    ```cpp
    --8<-- "code/DC/FFT/P3803_1.cpp"
    ```

### 位逆序置换

在 FFT 中，位逆序置换是一个重要的步骤。考虑递归过程，在每次递归会两两分组。

以$8$项多项式为例，模拟拆分的过程：

1. 初始序列为 $\{x_0, x_1, x_2, x_3, x_4, x_5, x_6, x_7\}$
2. 一次二分之后 $\{x_0, x_2, x_4, x_6\},\{x_1, x_3, x_5, x_7 \}$
3. 两次二分之后 $\{x_0,x_4\} \{x_2, x_6\},\{x_1, x_5\},\{x_3, x_7 \}$
4. 三次二分之后 $\{x_0\}\{x_4\}\{x_2\}\{x_6\}\{x_1\}\{x_5\}\{x_3\}\{x_7 \}$

规律：其实就是原来的序列每个数用二进制表示，然后把二进制**翻转**，就是最终位置的下标。

$$
\begin{aligned}
0 &\to 000 \to 000 \to 0 \\
1 &\to 001 \to 100 \to 4 \\
2 &\to 010 \to 010 \to 2 \\
3 &\to 011 \to 110 \to 6 \\
4 &\to 100 \to 001 \to 1 \\
5 &\to 101 \to 101 \to 5 \\
6 &\to 110 \to 011 \to 3 \\
7 &\to 111 \to 111 \to 7 \\
\end{aligned}
$$

在 $O(n\log n)$ 的时间复杂度内，可以将序列进行位逆序置换。

??? note "位逆序置换代码实现"

    !!! tip "位逆序置换优化"

        在`cpp`中也可以直接使用`bitset`来翻转二进制位，时间复杂度与位数有关。

    === "基本实现"

        该实现的时间复杂度为 $O(n\log n)$。

        ```cpp
        void bit_reverse(vector<complex<double>> &f) {
          int n = f.size();
          // i = 0...01, j = 10...0, 在二进制下是翻转的
          for (int i = 1, j = n / 2; i < n - 1; i++) {
            if (i < j) { swap(f[i], f[j]); }  // 互换位置，i < j 避免重复交换
            int k = n / 2;  // 保持二进制翻转
            while (j >= k) {
              j -= k;
              k /= 2;
            }
            if (j < k) { j += k; }
          }
        }
        ```
    
    === "优化实现"

        该实现的时间复杂度为 $O(n)$。

        ```cpp
        void bit_reverse(vector<complex<double>> &f) {
          int n = f.size();
          vector<int> reverse(n);
          for (int i = 0; i < n; ++i) {
            reverse[i] = reverse[i >> 1] >> 1;
            if ((i & 1) != 0) { reverse[i] |= n >> 1; }  // 如果最后一位是 1，则翻转成 n/2
          }
          for (int i = 0; i < n; ++i) {  // 保证每对数只翻转一次
            if (i < reverse[i]) { swap(f[i], f[reverse[i]]); }
          }
        }
        ```
### 蝶形操作

在 $\text{FFT}$ 中，每次递归都会两两分组，然后进行蝶形操作。

蝶形操作是一种两两合并的操作，每次合并两个数，然后乘上一个单位根。在蝶形操作中，每个数都会和另一个数进行一次乘法和一次加法。

???+ note "[【模板】多项式乘法（FFT）](https://www.luogu.com.cn/problem/P3803){target='_blank'}"

    ```cpp
    --8<-- "code/DC/FFT/P3803_2.cpp"
    ```

!!! tip 

    并行扫描算法的思路与蝶形变换类似

#### 应用
1. 多项式乘法 $h(x) = f(x) \times g(x)$
2. 多项式求逆 $g(x) = f(x)^{-1}$   
   $f(x) \times g(x) = 1$，$g(x) = f(x)^{-1}$，$g(x)$ 为 $f(x)$ 的逆元

## NTT

数论快速傅里叶变换（$\text{Number Theoretic Transform}$，$\text{NTT}$）是一种高效实现 $\text{DFT}$ 的算法。$\text{NTT}$ 算法的基本思想是将 $\text{DFT}$ 的复数域转化为模数域。

$\text{NTT}$ 通过将离散傅立叶变换化为 $F={\mathbb {Z}/p}$，整数模质数 $p$。这是一个有限域，只要 $n$ 可除 $p-1$，就存在本原 $n$ 次方根，所以有 $p=\xi n+1$。

具体来说，对于质数 $p=qn+1 (n=2^m)$，原根 $g$ 满足 $g^{qn} \equiv 1 \pmod p$, 将 $g_n=g^q\pmod p$ 看做 $\omega_n$ 的等价，则其满足相似的性质，比如 $g_n^n \equiv 1 \pmod p, g_n^{n/2} \equiv -1 \pmod p$。



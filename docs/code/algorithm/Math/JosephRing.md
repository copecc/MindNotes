---
title: 约瑟夫环
tags:
  - 约瑟夫环
  - Joseph Ring
---

# 约瑟夫环

给定 $n$ 个人围成一圈，从第 $1$ 个人开始报数，报到 $m$ 的人出列，问最后剩下的是谁？  
解决方法：  

1. 朴素模拟：时间复杂度 $O(n*m)$，空间复杂度 $O(n)$
2. 数学归纳法：时间复杂度 $O(n)$，空间复杂度 $O(1)$

## 数学归纳法
假设 $f(n, m)$ 表示 $n$ 个人报数到 $m$ 出列时，幸存者的编号(从 $0$ 开始)。  
当第 $1$ 个人出列时（编号为 $m-1$），剩下 $n-1$ 个人。新一轮编号从原编号为 $m$ 的人开始，这个人在新一轮的编号为 $0$，原编号为 $m+1$ 的人在新一轮的编号为 $1$，依此类推。  
因此，旧编号与新编号的关系为：$\text{old\_index} = (\text{new\_index} + m) \% n$  

递推关系如下：  

$$
f(n,m) = 
\begin{cases}
0, & n=1 \\
\left(f\left(n-1,m\right) + m\right) \% n, & n>1
\end{cases}
$$

!!! tip
    当递推关系呈现先不断递增，然后骤减到 $0$ 的形式时，可以考虑取模运算。

???+ note "[约瑟夫环](https://www.luogu.com.cn/problem/P8671){target=_blank}"

    ```cpp
    #include <iostream>
    using namespace std;

    int64_t josephus(int64_t n, int64_t m) {
      int64_t result = 0;  // 只有一个人时, 编号为0
      for (int64_t i = 2; i <= n; ++i) { result = (result + m) % i; }
      return result;
    }

    int main() {
      ios::sync_with_stdio(false);
      cin.tie(nullptr);
      cout.tie(nullptr);
      int64_t n, m;
      cin >> n >> m;
      cout << josephus(n, m) + 1 << "\n";  // 从1开始编号
      return 0;
    }
    ```

## 变种约瑟夫环

每轮报数的值不一样，由数组 $ms$ 给出，$ms[i]$ 表示第 $i$ 轮报数的值，则递推关系为:  

$$
f(n, ms) = 
\begin{cases}
0, & n=1 \\
\left(f\left(n-1, ms\right) + ms[n-2]\right) \% n, & n>1
\end{cases}
$$  

???+ note "[变种约瑟夫环](){target=_blank}"

    ```cpp
    #include <cstdint>
    #include <iostream>
    #include <vector>
    using namespace std;

    int64_t josephus(int64_t n, const vector<int64_t> &ms) {
      int64_t result = 0;  // 只有一个人时, 下标为0
      for (int64_t i = 2; i <= n; ++i) { result = (result + ms[i - 2]) % i; }
      return result;
    }

    int main() {
      ios::sync_with_stdio(false);
      cin.tie(nullptr);
      cout.tie(nullptr);
      int64_t n;
      cin >> n;
      vector<int64_t> ms(n - 1);
      for (int64_t i = 0; i < n - 1; ++i) { cin >> ms[i]; }
      cout << josephus(n, ms) + 1 << "\n";  // 从1开始编号
      return 0;
    }
    ```
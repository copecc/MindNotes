---
title: 树状数组
tags:
  - 树状数组
  - BIT
---

# 树状数组

树状数组（$\text{Binary Indexed Tree} / \text{Fenwick Tree}$）用于维护前缀累积信息（如前缀和、前缀异或、频率累加等）。  
若运算可逆且满足结合（如加法、异或或模下可逆的乘法），也可用于相应的区间/差分场景。

!!! tip "树状数组的下标"

    数组的下标从 $1$ 开始编号（$\text{one-based indexing}$）

## lowbit
$lowbit$ 取一个正整数二进制表示中最低位的 $1$（最低有效的 $1$ 对应的值），这是因为 $-x$ 的二进制表示是 $\sim x + 1$，即对 $x$ 取反后加 $1$，因此 $x \And -x$ 会保留 $x$ 的最低位的 $1$，其余位均为 $0$。  
$lowbit$ 只对正整数有意义（取二进制表示中最低的 $1$）。$x=0$ 时没有任何位为 $1$，因此从语义上说 $lowbit(0)$ 是无意义的。  

???+ notes "$lowbit$"

    ```cpp
    int64_t lowbit(int64_t x) { return x & (-x); }
    ```

## 单点更新、区间查询
维护一个数组 $tree$，其中 $tree[i]$ 存储原数组中从 $i - lowbit(i) + 1$ 到 $i$ 的区间和（或其他可逆结合运算的结果）。  
前缀和 $sum(1, pos)$ 可以通过不断减去 $lowbit$ 来累加 $tree$ 中的值实现。  
单点更新时，需要将更新值加到所有包含该位置的 $tree$ 元素中，这可以通过不断加上 $lowbit$ 来实现。

??? note "[【模板】树状数组 1](https://www.luogu.com.cn/problem/P3374){target=_blank}"

    ```cpp
    --8<-- "code/DS/BIT/P3374.cpp"
    ```

## 区间更新、单点查询

1. 维护差分数组 $D$: $D[1]=A[1], D[i]=A[i]-A[i-1] (i>1)$  
2. 区间 $[l,r]$ 加 $val$ 等价于 $D[l]+=val, D[r+1]-=val$（两次点增）  
3. 查询 $A[i]$ 则为 $\sum_{j=1}^i D[j]$（前缀和）

??? note "[【模板】树状数组 2](https://www.luogu.com.cn/problem/P3368){target=_blank}"

    ```cpp
    --8<-- "code/DS/BIT/P3368.cpp"
    ```

 区间更新、区间查询

1. 区间更新同上，维护差分数组 $D$
2. 区间查询  
    1. 单点的值: $A[i]=\sum_{j=1}^i D[j]$  
    2. 区间和: $\begin{aligned}
\sum_{i=1}^r A[i] &= \sum_{i=1}^r \sum_{j=1}^i D[j] = rD[1] + (r-1)D[2] + \ldots + (r-(r-1))D[r] \\
&=r\sum_{i=1}^r D[i] - \sum_{i=1}^r (i-1)D[i]\end{aligned}$  
    1. 令 $F[i]=(i-1)D[i]$，则 $\sum_{i=l}^r A[i]=r\sum_{i=1}^r D[i]-\sum_{i=1}^r F[i]-((l-1)\sum_{i=1}^{l-1} D[i]-\sum_{i=1}^{l-1} F[i])$  
1. 维护两个树状数组，一个维护 $D$ 用于区间更新，一个维护 $F$ 用于区间查询，其中 $F[i]=(i-1)*D[i]$，则 $F[i]$ 的区间更新等价于 $D[i]$ 的区间更新乘以 $(i-1)$

!!! tip "区间更新、区间查询"
    区间查询常用前缀和的差值实现，因此需要将区间查询转化为前缀和的形式  
    区间更新常用差分数组的形式实现，因此需要将区间更新转化为差分数组的形式  
    线段树也可以实现区间更新和区间查询  

??? note "[【模板】线段树 1](https://www.luogu.com.cn/problem/P3372){target=_blank}"

    ```cpp
    --8<--  "code/DS/BIT/P3372.cpp"
    ```

## 二维单点更新、区间查询

??? note "[二维区域和检索 - 可变](https://leetcode.cn/problems/range-sum-query-2d-mutable/){target=_blank}"

    ```cpp
    --8<--  "code/DS/BIT/range-sum-query-2d-mutable.cpp"
    ```

## 二维区间更新、区间查询

维护四个二维树状数组，分别维护 $D, i*D, j*D, i*j*D$，其中 $D$ 为差分数组，$D[i][j]=A[i][j]-A[i-1][j]-A[i][j-1]+A[i-1][j-1]$。

??? note "[上帝造题的七分钟](https://www.luogu.com.cn/problem/P4514){target=_blank}"

    ```cpp
    --8<--  "code/DS/BIT/P4514.cpp"
    ```
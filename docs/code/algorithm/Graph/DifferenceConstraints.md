---
title: 差分约束系统
tags:
  - 差分约束系统
  - Difference Constraints
---

# 差分约束系统

差分约束系统是一类特殊的线性不等式系统，形式为 $x_j - x_i \leq c_k$，其中 $x_i$ 和 $x_j$ 是变量，$c_k$ 是常数。差分约束系统可以用来描述变量之间的相对关系，并且可以通过图论的方法来求解。

差分约束系统的求解通常涉及将不等式转化为图的边权，然后使用最短路径算法（如 $\text{Bellman-Ford}$ 或 $\text{SPFA}$）来检测负环，从而判断系统是否有解。

## 差分约束系统的图论表示与求解

1. 差分约束系统的表示：设有 $n$ 个变量 $x_1, x_2, \ldots, x_n$ 和 $m$ 个差分约束 $x_j - x_i \leq c_k$。
2. 图的构建：

      1. 创建一个包含 $n$ 个节点的有向图，每个节点对应一个变量 $x_i$
      2. 对于每个差分约束 $x_j - x_i \leq c_k$，在图中添加一条从节点 $i$ 到节点 $j$ 的有向边，边权为 $c_k$（$x_j \leq x_i + c_k$ 的形式与图的最短距离形式类似）

3. 判断图中是否存在负权环：
      1. 如果存在负权环，则差分约束系统无解
      2. 如果不存在负权环，则差分约束系统有解

???+ note "[【模板】差分约束](https://www.luogu.com.cn/problem/P5960){target=_blank}"

    ```cpp
    --8<-- "code/Graph/DC/P5960.cpp"
    ```

## 约束形式的扩展

1. 原始约束：$x_i - x_j \geq c_k$（大于等式形式）

    可以采取以下两种方式处理：

    - $x_j - x_i \leq -c_k$
    - 转化为判定无限增加的环路（正环）(1)
          {.annotate}

          1. 需要修改 $\text{SPFA}$ 逻辑，距离变大才更新

2. 原始约束： $x_i - x_j = c_k$（等式约束，特别地, 当 $c_k = 0$ 时表示相等）

    改为添加两条约束：

    - $x_i - x_j \geq c_k \Longleftrightarrow x_j - x_i \leq -c_k$
    - $x_i - x_j \leq c_k \Longleftrightarrow x_i - x_j \leq c_k$ (1)
          {.annotate}

          1. 已经是差分约束形式, 直接添加边即可

3. 原始约束：$x_i \in [L, R]$（变量范围约束）

    添加虚拟节点 $x_{n+1}$（限制超级源点），添加两条约束：

    - $x_i - x_{n+1} \geq L \Longleftrightarrow x_{n+1} - x_i \leq -L$（即从 $x_i$ 到 $x_{n+1}$ 的边权为 $-L$）
    - $x_i - x_{n+1} \leq R \Longleftrightarrow x_i - x_{n+1} \leq R$（即从 $x_{n+1}$ 到 $x_i$ 的边权为 $R$） (1)
          {.annotate}

          1. $x_{n+1}$ 可以看作一个参考点，所有变量相对于这个点的偏移量都在 $[L, R]$ 范围内

4. 原始约束：$x_i$ 是自由变量

    添加虚拟节点 $S$（连通超级源点），添加以下约束：

    - $x_i - S \leq 0$ (即从 $S$ 到 $x_i$ 的边权为 0)

5. 原始约束：$x_i - x_j < c_k$（严格不等式）

    转化为非严格不等式：

    1. 整数变量：$x_i - x_j \leq c_k - 1$
    2. 实数变量：$x_i - x_j \leq c_k - \epsilon$，其中 $\epsilon$ 是一个很小的正数 (1)
          {.annotate}

          1. 需要考虑精度问题，通常减去一个很小的值 $\epsilon$ (如 $1e-9$)

    如果所有变量 $x_i$ 都是整数，则可以通过引入新的变量和约束来处理严格不等式。

6. 单调关系约束：变量之间隐含了顺序关系(如 $x_1 \leq x_2 \leq x_3 \leq \dots \leq x_n$)

    添加相应的约束：

    $x_i - x_{i-1} \geq 0 \Longleftrightarrow x_{i-1} - x_i \leq 0$

7. 原始约束：$x_i / x_j \leq c_k$ ($c_k > 0$)

    转化为对数形式再处理：

    $\log x_i - \log x_j \leq \log c_k$

!!! tip "注意事项"

    - 连通超级源点 $S$ 与 限制超级源点 $x_{n+1}$ 必须区分
    - 连通超级源点用于检测全局负环，权重 $0$，保证每个节点可达
    - 限制超级源点用于限制变量范围或保证非负性
    - 严格不等式需谨慎处理整数与实数情况

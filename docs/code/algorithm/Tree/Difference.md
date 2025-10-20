---
title: 树上差分
tags:
  - 差分
  - Difference
---

# 树上差分

树上差分是指在树的路径进行差分操作，以便高效地处理路径更新和查询问题。树上差分可分为点差分和边差分两种。

## 点差分

树上点差分基本思想是通过差分操作记录路径上的修改值（针对节点），最后通过一次深度优先搜索（$\text{DFS}$）将差分值累加，从而得到每个节点的最终值。具体步骤如下：

1. 使用树上倍增或者 $\text{Tarjan}$ 算法预处理每条查询中路径两端点的最低公共祖先$\text{LCA}$(1)
    {.annotate}

    1. 见 [最近公共祖先](./LCA.md)

2. 对于每条路径 $u \longleftrightarrow v$, 找到端点的最低公共祖先 $\text{LCA}$
      - 在 $u$ 和 $v$ 上加上修改值
      - 在 $\text{LCA}$ 上减去修改值
      - 如果 $\text{LCA}$ 不是根节点，那么在 $\text{LCA}$ 的父节点上也减去修改值
3. 最后对树进行一次 $\text{DFS}$, 将差分值进行累加, 即可得到每个节点的最终值


???+ note "[Max Flow P](https://www.luogu.com.cn/problem/P3128){target=_blank}"

    给定一棵有 $n$ 个节点的树和 $k$ 条路径，每条路径连接树上的两个节点。初始时每个节点的值为 $0$。对于每条路径，路径上所有节点的值都加 $1$。请你计算经过所有路径操作后，树上节点的最大值。

    ```cpp
    --8<-- "code/Tree/Difference/P3128.cpp"
    ```

## 边差分

树上边差分基本思想是通过差分操作记录路径上的修改值（针对边），最后通过一次深度优先搜索（$\text{DFS}$）将差分值累加，从而得到每条边的最终值。具体步骤如下：

1. 使用树上倍增或者 $\text{Tarjan}$ 算法预处理每条查询中路径两端点的最低公共祖先$\text{LCA}$(1)
    {.annotate}

    1. 见 [最近公共祖先](./LCA.md)

2. 对于每条路径 $u \longleftrightarrow v$, 找到端点的最低公共祖先 $\text{LCA}$
      - 在 $u$ 和 $v$ 上加上修改值
      - 在 $\text{LCA}$ 上减去两倍的修改值
3. 最后对树进行一次 $\text{DFS}$, 假设边 $e$ 从父节点 $u$ 连向子节点 $v$，令 $\text{weight}[e] += \text{num}[v]$ 即可得到每条边的最终值。然后将点权累加到父节点上。

??? note "[Network](http://poj.org/problem?id=3417){target=_blank}"

    给定一棵含 $N$ 个节点的树，和 $M$ 条新增边。每次删除一条原树边和一条新增边，求使网络变为不连通的方案数。

    ??? hint

        每条新增边在树上对应一条路径。若某树边被 $k$ 条新增边跨过，则删掉它后仍连通的新边有 $k$ 条。

        用树上差分 + $\text{LCA}$ 统计每条树边被跨过的次数 $k$，
        答案为对每条树边求贡献：

        1. $k = 0 \Rightarrow +M$
        2. $k = 1 \Rightarrow +1$
        3. $k > 1 \Rightarrow +0$

    !!! warning "数据范围"

        $1 \leq N \leq 10^5$, $1 \leq M \leq 10^5$

        {--卡常严重？--}

        似乎`vector`会TLE，改用链式前向星；`cin/cout`也会TLE，改用`scanf`/`printf`

    ```cpp
    --8<-- "code/Tree/Difference/POJ3417.cpp"
    ```
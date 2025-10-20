---
title: 最小生成树
tags:
  - Minimum Spanning Tree
  - MST
---

# 最小生成树

最小生成树（$\text{Minimum Spanning Tree}, \text{MST}$）是指在一个连通加权无向图中，选取一部分边，使得所有节点都连通且边的权重之和最小的生成树。常用的算法有 $\text{Kruskal}$ 算法和 $\text{Prim}$ 算法。

## Kruskal算法

$\text{Kruskal}$ 算法的基本思想是将所有边按权重从小到大排序，然后依次选择边，若选择该边不会形成环，则将其加入生成树中，直到生成树包含所有节点为止。通常使用并查集（$\text{Union-Find}$）来检测环的形成。

???+ note "[【模板】最小生成树](https://www.luogu.com.cn/problem/P3366){target=_blank}"

    给定一个包含 $n$ 个节点和 $m$ 条边的无向图，求该图的最小生成树的权值和。如果图不连通，则输出 "orz"。

    ```cpp
    --8<-- "code/Graph/MST/P3366_1.cpp"
    ```

## Prim算法

$\text{Prim}$ 算法的基本思想是从一个节点开始，逐步扩展生成树，每次选择一条连接生成树和非生成树的边中权重最小的边，将其加入生成树，直到所有节点都包含在生成树中。通常使用优先队列（$\text{Priority Queue}$）来高效地选择最小权重边。

???+ note "[【模板】最小生成树](https://www.luogu.com.cn/problem/P3366){target=_blank}"

    ```cpp
    --8<-- "code/Graph/MST/P3366_2.cpp"
    ```
    {.annotate}

    1. 避免重复插入同一节点到优先队列中，确保每个节点只会被加入生成树一次。也可以省略，常数时间会稍微高一些。
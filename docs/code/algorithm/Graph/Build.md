---
title: 建图
tags:
  - Build Graph
---

# 建图

## 链式前向星

链式前向星是一种高效的图的存储结构，适用于稀疏图。它通过三个数组来表示图的边：$head$、$to$ 和 $next$。

- $head$ 数组存储每个节点的出边链表的头节点。
- $to$ 数组存储每条边的终点节点。
- $next$ 数组存储同一起点的下一条边。

???+ note "链式前向星建图"

    ```cpp
    const int MAX_NODES = 10'005;     // 图的最大节点数
    const int MAX_EDGES = 20'005;     // 图的最大边数, 有向图为边数, 无向图为边数的2倍

    vector<int> head(MAX_NODES, -1);  // 节点i的第一条边在edges中的下标
    vector<int> next(MAX_EDGES);      // 与edges[i]同起点的下一条边在edges中的下标
    vector<int> to(MAX_EDGES);        // edges[i]的终点
    vector<int> weight(MAX_EDGES);    // edges[i]的权重
    int edge_count = 0;               // 当前边数

    void init_graph(int n) {
      edge_count = 0;
      fill(head.begin(), head.begin() + n, -1);
    }

    // 添加边. 如果是无向图, 需要分别对u->v和v->u调用add_edge
    void add_edge(int u, int v, int w = 1) {
      to[edge_count]     = v;           // 添加一条从u到v的边
      next[edge_count]   = head[u];     // 将新边的下一条边指向当前u的第一条边
      head[u]            = edge_count;  // 更新u的第一条边为新添加的边
      weight[edge_count] = w;           // 设置边的权重
      ++edge_count;                     // 增加边计数
    }

    // 遍历节点u的所有出边
    void traverse_edges(int u) {
      for (int i = head[u]; i != -1; i = next[i]) {  // 遍历u的所有出边
        int v = to[i];
        // 对边(u, v)执行操作
      }
    }
    ```
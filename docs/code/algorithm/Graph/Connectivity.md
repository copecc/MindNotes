---
title: 连通性
tags:
  - 强连通分量
  - Strongly Connected Component
  - SCC
  - 割点
  - Articulation Point
  - 桥
  - Bridge
  - Tarjan
---

# 连通性

图的连通性指的是图中顶点之间的可达性。

在无向图中，如果任意两个顶点之间存在路径相互可达，则称该图为连通图；否则称为非连通图。

在有向图中，如果任意两个顶点之间存在路径相互可达，则称该图为强连通图；如果有向图的基图（将有向边视为无向边）是连通的，则称该图为弱连通图；否则称为非连通图。

## 强连通分量

强连通分量（$\text{Strongly Connected Component}$, $\text{SCC}$）是指在有向图中，任意两个顶点之间都存在路径相互可达的最大子图。$\text{Tarjan}$ 算法可以在线性时间内找到图中的所有强连通分量。

$\text{Tarjan}$ 算法求解强连通分量的核心思想是使用深度优先搜索（$\text{DFS}$）遍历图，并利用时间戳（$timer$）和栈来跟踪当前路径上的节点。每个节点在被访问时会被赋予一个唯一的时间戳（$dfn$），同时维护一个低链接值（$low$），表示从该节点出发能够到达的最早节点的时间戳。当遍历完成后，如果某个节点的时间戳等于其低链接值，则说明该节点是一个强连通分量的根节点，可以将栈中的节点弹出，形成一个强连通分量。

$\text{Tarjan}$ 算法求得的强连通分量具有以下性质：

- 每个强连通分量都是图的最大子图，且任意两个节点之间都存在路径相互可达。
- 强连通分量之间的关系可以形成一个有向无环图（$\text{DAG}$），即强连通分量之间不存在环路。
- 强连通分量的划分是唯一的，即对于同一个有向图，无论使用何种方法求解，得到的强连通分量划分都是相同的。
- 强连通分量编号按照拓扑排序的逆序进行编号，即编号较小的强连通分量在拓扑排序中位于后面。

???+ note "[强连通分量](https://www.luogu.com.cn/problem/B3609){target=_blank}"

    给定一个有向图，求出图中的所有强连通分量。输出强连通分量的个数以及每个强连通分量中的点，要求每个强连通分量内的点按字典序排序，强连通分量之间也按字典序排序。

    ```cpp
    --8<-- "code/Graph/Connectivity/B3609.cpp"
    ```

利用强连通分量可以将有向图进行缩点，得到一个有向无环图（$\text{DAG}$）。在缩点后的图中，每个强连通分量被视为一个单独的节点，强连通分量之间的边则表示原图中不同强连通分量之间的连接关系。缩点后的有向无环图具有以下性质：

- 缩点后的图中不存在环路，因为强连通分量之间的连接关系是单向的。
- 可以在缩点后的图上进行拓扑排序，从而确定强连通分量之间的依赖关系。
- 可以在缩点后的图上进行动态规划等算法，从而解决原图中的一些复杂问题。

??? note "[【模板】缩点](https://www.luogu.com.cn/problem/P3387){target=_blank}"

    给定一个有向图，每个点有一个权值。求从某个点出发，经过若干条边后所能获得的最大权值和。经过的点的权值只计算一次。

    ```cpp
    --8<-- "code/Graph/Connectivity/P3387.cpp"
    ```
    ??? note

        1. 完成 $\text{DAG}$ 的构建后最好去重, 因为可能有多条边连向同一个SCC。如果不影响答案不去重也没关系, 只是会多做一些重复计算，影响效率。或者使用 `set` 代替 `vector` 存储边。

            ```cpp
            for (int i = 0; i < scc_count; ++i) {
              sort(dag[i].begin(), dag[i].end());
              dag[i].erase(unique(dag[i].begin(), dag[i].end()), dag[i].end());
            }
            ```
        
        2. 在缩点后的 $\text{DAG}$ 上可以使用拓扑排序加动态规划（$\text{DP}$）的方法求解最大权值和问题。也可以使用深度优先搜索（$\text{DFS}$）的记忆化搜索方法实现。

            ```cpp
            vector<int> visit_dag(scc_count);
            auto dfs = [&](auto &&self, int u) -> int {
              if (visit_dag[u]) { return dp[u]; }
              visit_dag[u] = 1;
              for (int v : dag[u]) {
                dp[u] = max(dp[u], self(self, v) + scc_weight[u]);
              }
              return dp[u];
            };

            for (int i = 0; i < scc_count; ++i) {
              answer = max(answer, dfs(dfs, i));
            }
            ```

??? note "[受欢迎的牛 G](https://www.luogu.com.cn/problem/P2341){target=_blank}"

    给定一个有向图，求图中是否存在一个强连通分量，使得所有其他强连通分量都可以通过有向边到达该强连通分量。如果存在这样的强连通分量，输出该强连通分量中节点的数量；否则输出 0。

    ```cpp
    --8<-- "code/Graph/Connectivity/P2341.cpp"
    ```

## 割点

割点（$\text{Articulation Point}$）是指在无向图中，去掉该节点及其相关的边后，图的连通分量数量增加的节点。$\text{Tarjan}$ 算法可以在线性时间内找到图中的所有割点。

割点的判定条件如下：

- 对于非根节点 $u$，如果存在一个子节点 $v$，使得从 $v$ 出发无法通过其他路径回到 $u$ 或 $u$ 的祖先节点（即 $low[v] \geq dfn[u]$），则 $u$ 是割点。
- 对于根节点 $u$，如果 $u$ 有两个或以上的子节点，则 $u$ 是割点。

$\text{Tarjan}$ 算法求解割点的核心思想是使用深度优先搜索（$\text{DFS}$）遍历图，并利用时间戳（$timer$）和低链接值（$low$）来跟踪节点的访问情况。在遍历过程中，根据上述判定条件识别割点。

???+ note "[【模板】割点（割顶）](https://www.luogu.com.cn/problem/P3388){target=_blank}"

    ```cpp
    --8<-- "code/Graph/Connectivity/P3388.cpp"
    ```

## 桥

桥（$\text{Bridge}$）是指在无向图中，去掉该边后，图的连通分量数量增加的边。$\text{Tarjan}$ 算法可以在线性时间内找到图中的所有桥。

桥的判定条件如下：

- 对于一条边 $(u, v)$，如果从节点 $v$ 出发无法通过其他路径回到节点 $u$ 或 $u$ 的祖先节点（即 $low[v] > dfn[u]$），则该边是桥。

???+ note "$\text{Tarjan}$ 算法求解桥"

    ```cpp
    vector<int> dfn(n + 1);  // 遍历到该点的时间戳 depth first number
    vector<int> low(n + 1);  // 从该点出发所能到达的最小时间戳
    vector<pair<int, int>> bridge;

    int timer   = 0;
    auto tarjan = [&](auto &&self, int u, int parent) -> void {
      dfn[u] = low[u] = ++timer;
      for (int v : g[u]) {
        if (!dfn[v]) {
          self(self, v, u);
          if (low[v] > dfn[u]) { bridge.emplace_back(u, v); }
          low[u] = min(low[u], low[v]);
        } else if (v != parent) {
          low[u] = min(low[u], dfn[v]);
        }
      }
    };

    for (int i = 0; i < n; i++) {
      if (dfn[i] == 0) { tarjan(tarjan, i, -1); }
    }
    ```

??? note "[Network](https://acm.hdu.edu.cn/showproblem.php?pid=2460){target=_blank}"

    给定一个无向图，图中有 $n$ 个节点和 $m$ 条边。然后进行 $q$ 次操作，每次操作增加一条边，询问当前图中桥的数量。

    ```cpp
    --8<-- "code/Graph/Connectivity/HDU2460.cpp"
    ```
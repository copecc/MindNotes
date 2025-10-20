---
title: 最近公共祖先
tags:
  - 最近公共祖先
  - LCA
  - Lowest Common Ancestor
---

# 最近公共祖先

最近公共祖先（$\text{Lowest Common Ancestor}，\text{LCA}$）是指在一棵树中，两个节点的最近的共同祖先节点。如果两个节点中有一个是另一个的祖先节点，那么这个祖先节点就是它们的 $\text{LCA}$。

## 倍增法

倍增法通过预处理树的节点信息，使得每次查询LCA的时间复杂度降到 $O(\log N)$，其中 $N$ 是树的节点数。

倍增法的核心思想是预先计算每个节点的 $2^i$ 级祖先，并存储在一个二维数组中。这样，在查询两个节点的 $\text{LCA}$ 时，可以通过不断提升节点到其祖先来找到它们的共同祖先。

???+ note "预处理 $\text{LCA}$"

    根据给定树的父节点数组 $parent$ 或邻接表 $tree$，预处理出每个节点的 $2^i$ 级祖先，并存储在一个二维数组中。

    === "父节点数组"

        $parent[i]$ 表示节点 $i$ 的父节点， 根节点的父节点为 $-1$。  
        如果需要得到节点 $u$ 的 $2^j$ 级祖先，可以先找到节点 $u$ 的 $2^{j-1}$ 级祖先，然后再找到该祖先的 $2^{j-1}$ 级祖先。

        ```cpp
        vector<vector<int>> prepare_LCA(const vector<int> &parent) {
          int n = parent.size();
          int m = 32 - __builtin_clz(n);  // 祖先的最大深度(2^m >= n) (1)
          vector<vector<int>> st(n, vector<int>(m, -1));  // st[i][j]表示节点i的2^j级祖先

          // 初始化第2^0个祖先, 即父节点
          for (int i = 0; i < n; ++i) { st[i][0] = parent[i]; }
          // 递推计算第2^j个祖先
          for (int j = 1; j < m; ++j) {
            for (int i = 0; i < n; ++i) {
              if (st[i][j - 1] != -1) { st[i][j] = st[st[i][j - 1]][j - 1]; }
            }
          }

          return st;
        }
        ```

        1. $64$ 位数据: `#!c++ int64_t m = 64 - __builtin_clzll(n);`
   
    === "邻接表"

        $tree[i]$ 表示节点 $i$ 的所有子节点。  
        预处理时需要通过 $DFS$ 计算每个节点的父节点，然后再通过父节点数组计算每个节点的 $2^i$ 级祖先。

        ```cpp
        vector<vector<int>> prepare_LCA(const vector<vector<int>> &tree, int root = 0) {
          int n = tree.size();
          int m = 32 - __builtin_clz(n);  // 祖先的最大深度(2^m >= n) (1)
          vector<vector<int>> st(n, vector<int>(m, -1));  // st[i][j]表示节点i的2^j级祖先

          auto dfs = [&](auto &&self, int x, int from) -> void {
            st[x][0] = from;         // 初始化第2^0个祖先, 即父节点
            for (int y : tree[x]) {
              if (y != from) {
                self(self, y, x);
              }
            }
          };
          dfs(dfs, root, -1);  // 计算每个节点的父节点
          // 递推计算第2^j个祖先
          for (int j = 1; j < m; ++j) {
            for (int i = 0; i < n; ++i) {
              if (st[i][j - 1] != -1) { st[i][j] = st[st[i][j - 1]][j - 1]; }
            }
          }

          return st;
        }

        ```

        2. $64$ 位数据: `#!c++ int64_t m = 64 - __builtin_clzll(n);`


!!! tip "查询节点 $u$ 的 $k$ 级祖先"

    如果要查询节点 $u$ 的 $k$ 级祖先，可以将 $k$ 分解为二进制形式 $k = b_mb_{m-1}...b_1b_0$，然后通过不断提升节点到其 $2^i$ 级祖先来实现。

    !!! example

        $k = 13$，则可以表示为 $1101_2$，即 $2^3 + 2^2 + 2^0$。因此可以通过三次提升操作来找到节点 $u$ 的 $13$ 级祖先，即先提升到 $2^0$ 级祖先，再从该祖先提升到 $2^2$ 级祖先，最后再提升到 $2^3$ 级祖先。
    

??? note "[树节点的第 K 个祖先](https://leetcode.cn/problems/kth-ancestor-of-a-tree-node/description/){target=_blank}"

    给定一棵树的节点数 $n$ 和一个数组 $parent$，其中 $parent[i]$ 是节点 $i$ 的父节点。根节点的父节点为 $-1$。  
    设计一个数据结构来实现查询节点 $u$ 的第 $k$ 个祖先节点的功能。如果不存在这样的祖先节点，返回 $-1$。

    ```cpp
    class TreeAncestor {
     public:
      TreeAncestor(int n, vector<int>& parent) {
        int m = 32 - __builtin_clz(n);
        st.resize(n, vector<int>(m, -1));
        for (int i = 0; i < n; ++i) { st[i][0] = parent[i]; }
        for (int j = 1; j < m; ++j) {
          for (int i = 0; i < n; ++i) {
            if (st[i][j - 1] != -1) { st[i][j] = st[st[i][j - 1]][j - 1]; }
          }
        }
      }

      int getKthAncestor(int node, int k) { // (1)!
        for (; (k != 0) && (node != -1); k &= k - 1) { node = st[node][__builtin_ctz(k)]; }
        return node;
      }

     private:
      vector<vector<int>> st;
    };
    ```

    1. `#!c++ __builtin_ctz(k)`（$\text{count tailing zero}$） 返回整数 $k$ 的二进制表示中从最低位开始连续的 $0$ 的个数，即 $k$ 中最低位的 $1$ 所在的位置（从 $0$ 开始计数）。

        !!! example
            `#!c++ __builtin_ctz(12)` 返回 $2$，因为 $12$ 的二进制表示为 $1100_2$，最低位的 $1$ 在位置 $2$。

        `#!c++ k &= k - 1` 这行代码的作用是将整数 $k$ 的二进制表示中最低位的 $1$ 变为 $0$，从而实现对 $k$ 的逐位处理。


!!! tip "查询节点 $u$ 和 $v$ 的 $\text{LCA}$"
    
    如果节点 $u$ 和 $v$ 在相同深度，可以通过不断提升节点 $u$ 和 $v$ 到其 $2^i$ 级祖先，直到它们的祖先节点相同为止。

    如果节点 $u$ 和 $v$ 在不同深度，可以先将较深的节点提升到与较浅节点相同的深度（即查询较深节点的 $k = |depth(u) - depth(v)|$ 级祖先），然后再进行上述操作。

    查询 $\text{LCA}$ 需要计算节点的深度 $depth$，因此在预处理时需要通过 $DFS$ 计算每个节点的深度。

??? note "[【模板】最近公共祖先（LCA）](https://www.luogu.com.cn/problem/P3379){target=_blank}"

    给定一棵有 $N$ 个节点的树和 $M$ 个查询，每个查询包含两个节点 $x$ 和 $y$，要求找出这两个节点的 $\text{LCA}$。树的根节点为 $S$，节点编号从 $1$ 到 $N$。


    ```cpp
    --8<-- "code/Tree/LCA/P3379_1.cpp"
    ```

## Tarjan 离线算法

$\text{Tarjan}$ 离线算法是一种用于在树或图中高效地处理多个最近公共祖先（$\text{LCA}$）查询的算法。该算法利用并查集（$\text{Union-Find}$）数据结构来动态维护节点的连通性，从而在一次深度优先搜索（$\text{DFS}$）遍历中处理所有的 $\text{LCA}$ 查询。

$\text{Tarjan}$ 离线算法的主要步骤如下：

1. 为每个节点创建一个并查集，并将每个节点的祖先初始化为其自身。还需要一个数组来记录每个节点的访问状态（未访问、正在访问、已访问）
2. $DFS$：从树的根节点开始进行 $DFS$ 遍历。在访问每个节点时，执行以下操作：

      - 标记当前节点为正在访问状态
      - 对于当前节点的每个子节点，递归地进行 $DFS$ 调用
      - 在返回到当前节点后，将当前节点与其子节点在并查集中合并，并将当前节点设置为其祖先
      - 标记当前节点为已访问状态（实际实现时可以不区分正在访问和已访问）
      - 处理与当前节点相关的所有 $\text{LCA}$ 查询。如果查询的另一个节点已经被访问过，则使用并查集找到它们的共同祖先，并记录结果

3. 在 $DFS$ 遍历完成后，所有的 $\text{LCA}$ 查询结果都已经被记录下来，可以直接返回


??? note "[【模板】最近公共祖先（LCA）](https://www.luogu.com.cn/problem/P3379){target=_blank}"

    ```cpp
    --8<-- "code/Tree/LCA/P3379_2.cpp"
    ```

## 重链剖分

见[重链剖分（HLD）](./HLD.md)。
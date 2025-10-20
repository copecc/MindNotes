---
title: 树的重心
tags:
  - 重心
  - Centroid
---

# 树的重心

在一棵树中，选择某个节点并删除后，树会分为若干棵子树。统计这些子树的节点数，并记录其中的最大值。遍历树上的所有节点，使得这个最大值最小的节点被称为树的重心。

树的重心有以下性质：

1. 重心的唯一性：树的重心如果不唯一，则至多有两个，且这两个重心是相邻的
2. 子树大小限制：以树的重心为根时，所有子树的大小都不超过整棵树节点数的一半
3. 距离和最小性：树中所有点到某个点的距离和中，到重心的距离和是最小的。如果树有两个重心，则到这两个重心的距离和相等
4. 重心在合并树中的位置：将两棵树通过一条边相连形成一棵新的树，那么新树的重心一定在连接原来两棵树的重心的路径上
5. 重心的动态变化：在一棵树上添加或删除一个叶子节点时，树的重心最多只会移动一条边的距离

根据这些性质可以求解树的重心。

???+ note "树的重心"

    === "定义"

        以某个节点为根时，最大子树的节点数最少，那么这个节点是重心

        ```cpp
        vector<int> find_centroid(const vector<vector<int>> &tree, int root = 0) {
          int n = tree.size();
          vector<int> size(n);    // 子树节点数
          int best = n;           // 最小的最大子节点数
          vector<int> centroids;  // 重心节点, 最多两个

          std::function<void(int, int)> dfs = [&](int x, int from) {
            size[x]       = 1;
            int max_child = 0;  // 最大子节点数
            for (int y : tree[x]) {
              if (y != from) {
                dfs(y, x);
                size[x]   += size[y];
                max_child  = max(max_child, size[y]);
              }
            }
            max_child = max(max_child, n - size[x]);  // x的补树节点数
            if (max_child < best) {
              best      = max_child;
              centroids = {x};
            } else if (max_child == best) {
              centroids.push_back(x);
            }
          };
          dfs(root, -1);

          return centroids;
        }
        ```

    === "性质2"

        以某个节点为根时，每颗子树的节点数不超过总节点数的一半，那么这个节点是重心

        ```cpp
        vector<int> find_centroid(const vector<vector<int>> &tree, int root = 0) {
          int n = tree.size();
          vector<int> size(n);         // 子树节点数
          vector<int> max_subtree(n);  // 最大子树节点数
          vector<int> centroids;       // 重心节点, 最多两个

          std::function<void(int, int)> dfs = [&](int x, int from) {
            size[x] = 1;
            for (int y : tree[x]) {
              if (y != from) {
                dfs(y, x);
                size[x]        += size[y];
                max_subtree[x]  = max(max_subtree[x], size[y]);
              }
            }
            max_subtree[x] = max(max_subtree[x], n - size[x]);  // x的补树节点数
          };
          dfs(root, -1);
          for (int i = 0; i < n; ++i) {
            if (max_subtree[i] <= n / 2) { centroids.push_back(i); }
          }

          return centroids;
        }
        ```

    === "性质3"

        树中所有点到某个点的距离和中，到重心的距离和是最小的；如果有两个重心，那么到它们的距离和一样

        ```cpp
        vector<int> find_centroid(const vector<vector<int>> &tree, int root = 0) {
          int n = tree.size();
          vector<int> size(n);    // 子树节点数
          vector<int> dist(n);    // 以节点i为根时，所有节点到i的距离和
          vector<int> centroids;  // 重心节点, 最多两个

          // 计算以root为根时，所有节点到root的距离和
          std::function<void(int, int)> dfs1 = [&](int x, int from) {
            size[x] = 1;
            for (int y : tree[x]) {
              if (y != from) {
                dfs1(y, x);
                size[x] += size[y];
                dist[x] += dist[y] + size[y];  // dist[y] + size[y]: y的子树节点到x的距离和
              }
            }
          };
          dfs1(root, -1);
          // 换根dp, 计算以每个节点为根时，所有节点到该节点的距离和
          std::function<void(int, int)> dfs2 = [&](int x, int from) {
            for (int y : tree[x]) {
              if (y != from) {
                dist[y] = dist[x] - size[y] + (n - size[y]);
                dfs2(y, x);
              }
            }
          };
          dfs2(root, -1);
          int best = *min_element(dist.begin(), dist.end());
          for (int i = 0; i < n; ++i) {
            if (dist[i] == best) { centroids.push_back(i); }
          }
          
          return centroids;
        }
        ```


??? note "[Balancing Act](http://poj.org/problem?id=1655){target=_blank}"

    给定一棵有 `n` 个节点的树，求出它的重心节点编号以及删除该节点后，剩余子树中节点数最多的子树的节点数。

    ```cpp
    --8<-- "code/Tree/Centroid/POJ1655.cpp"
    ```
---
title: BFS
tags:
  - 广度优先搜索
  - Breadth First Search
  - 0-1 BFS
  - 双向 BFS
  - Bidirectional BFS
  - Meet in the Middle
---

# 广度优先搜索 (BFS)

广度优先搜索（$\text{Breadth First Search}，\text{BFS}$）是一种用于遍历或搜索树或图的算法。它从根节点（或任意选定的起始节点）开始，沿着树的宽度遍历节点，先访问所有邻近节点，然后再逐层向下访问更深层次的节点。

$\text{BFS}$ 的基本步骤如下：

1. 使用队列（Queue）来存储待访问的节点
2. 从起始节点开始，将其标记为已访问并加入队列
3. 重复以下步骤直到队列为空：
    - 从队列中取出一个节点，访问该节点
    - 将该节点的所有未访问过的邻近节点标记为已访问并加入队列
4. 继续上述过程，直到所有可达节点都被访问

???+ note "BFS"

    ```cpp
    vector<int> BFS(const vector<vector<int>> &graph, int start = 0) {
      int n = graph.size();
      vector<bool> visited(n, false);
      vector<int> distance(n, -1);  // 记录每个节点的深度

      queue<int> q;
      q.push(start);  // 从start节点开始入队
      visited[start] = true;
      int depth      = 0;

      while (!q.empty()) {
        int size = q.size();
        for (int i = 0; i < size; ++i) {
          int u = q.front();
          q.pop();
          distance[u] = depth;      // 记录节点u的深度
          for (int v : graph[u]) {  // 处理节点u
            if (!visited[v]) {
              visited[v] = true;
              q.push(v);  // 邻接点入队
            }
          }
        }
        ++depth;  // 深度增加
      }
      return distance;
    }
    ```

## 0-1 BFS

在某些图中，边的权重只有两种可能值（通常是 $0$ 和 $1$）。对于这种情况，可以使用一种称为$\text{0-1 BFS}$的特殊算法来高效地找到从起始节点到所有其他节点的最短路径。

$\text{0-1 BFS}$ 的基本思想是使用双端队列（$\text{Deque}$）来存储待访问的节点。对于每条边，如果其权重为 $0$，则将其终点添加到队列的前端；如果权重为 $1$，则将其终点添加到队列的后端。这样可以确保在处理节点时，总是优先处理权重较小的边。

这样做的好处是，$\text{0-1 BFS}$ 可以在 $O(V + E)$ 的时间复杂度内找到最短路径，其中 $V$ 是节点数，$E$ 是边数。这比传统的 $\text{Dijkstra}$ 算法在处理只有两种权重的图时更高效。

???+ note "[到达角落需要移除障碍物的最小数目](https://leetcode.cn/problems/minimum-obstacle-removal-to-reach-corner/description/){target=_blank}"

    给定一个二维网格 $grid$，其中 $0$ 表示空地，$1$ 表示障碍物。你可以从一个格子移动到其相邻的上、下、左、右四个格子。你需要找到从左上角 $(0, 0)$ 到右下角 $(m-1, n-1)$ 的路径上需要移除的障碍物的最小数目。

    ```cpp
    --8<-- "code/Graph/BFS/L2290.cpp"
    ```

??? note "[网格传送门旅游](https://leetcode.cn/problems/grid-teleportation-traversal/description/){target=_blank}"

    给定一个 $m \times n$ 的二维网格 $matrix$，每个格子包含一个字符，可能是大写字母（表示传送门）或 $\#$（表示障碍物）。你可以从左上角 $(0, 0)$ 出发，目标是到达右下角 $(m-1, n-1)$。你可以向上、下、左、右移动到相邻的格子，或者使用传送门将你传送到相同字母的格子。每次移动到相邻格子需要 $1$ 步，而使用传送门不需要任何步数，每种传送门只能使用一次。请返回从起点到终点的最少步数，如果无法到达则返回 $-1$。

    !!! tip "传送门"

        将二维网格中的每个格子视为图中的一个节点。所有相同字母（传送门）之间都有一条边权为 $0$ 的边，所有相邻格子之间都有一条边权为 $1$ 的边。

        传送门只能使用一次，因此需要一个额外的数组来记录每个传送门是否已经被使用过（或者使用传送门之后将该传送门对应的所有节点从图中删除）。

    ```cpp
    --8<-- "code/Graph/BFS/L3552.cpp"
    ```

## 双向 BFS

双向 $\text{BFS}$ 是一种优化的广度优先搜索算法，适用于在无权图中寻找两个节点之间的最短路径。与传统的 $\text{BFS}$ 从起始节点开始向外扩展不同，双向 $\text{BFS}$ 同时从起始节点和目标节点开始搜索，直到两次搜索相遇。

双向 $\text{BFS}$ 的好处在于它可以显著减少搜索空间，从而提高搜索效率。因为每次搜索的深度只有原来的一半，所以在大多数情况下，双向 $\text{BFS}$ 的时间复杂度为 $O(b^{d/2})$，其中 $b$ 是每个节点的平均分支因子，$d$ 是起始节点和目标节点之间的距离。

双向 $\text{BFS}$ 的核心思想和 $\text{Meet in the Middle}$ 相似，都是通过将问题分解为两个较小的子问题来提高效率。双向 $\text{BFS}$ 主要用于图的最短路径问题，$\text{Meet in the Middle}$ 主要用于组合问题（如子集和问题）。

??? note "[单词接龙](https://leetcode.cn/problems/word-ladder/description/){target=_blank}"

    给定两个单词（$\text{beginWord}$ 和 $\text{endWord}$）和一个字典 $\text{wordList}$，找出从 $\text{beginWord}$ 到 $\text{endWord}$ 的最短转换序列的长度。转换需遵循如下规则：

    - 每次转换只能改变一个字母。
    - 转换后的单词必须是字典中的单词。
    - 注意：$\text{beginWord}$ 不需要在字典中。

    ```cpp
    --8<-- "code/Graph/BFS/L127.cpp"
    ```
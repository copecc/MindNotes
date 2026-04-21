---
title: 图
tags:
  - hot100
  - 图
---

# 图

Hot100 里的图题主要覆盖 DFS/BFS、最短步数的多源扩展、拓扑排序和前缀树。这里把 `208` 一并放在本页，是为了和官方 Hot100 题组的收录方式保持一致。

## DFS 与 BFS

### 200. 岛屿数量

!!! problem "[200. 岛屿数量](https://leetcode.cn/problems/number-of-islands/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个由 `0` 和 `1` 组成的二维网格，其中 `1` 表示陆地、`0` 表示水域，请统计网格中岛屿的数量。岛屿由上下左右相邻的陆地格子组成，对角线不算连通。

    输入是一个二维字符网格，输出是岛屿总数。每个连通块只能计数一次，因此搜索时通常需要把已经访问过的陆地标记掉。

遍历网格时，一旦遇到 `1`，就把它所在的整块连通区域淹没掉。这样每个岛屿只会被计数一次，递归/队列负责把同一连通分量里的所有格子一次性标记完。

???+ solution "200. 岛屿数量"

    ```cpp
    --8<-- "code/leetcode/hot100/L200.cpp"
    ```

### 994. 腐烂的橘子

!!! problem "[994. 腐烂的橘子](https://leetcode.cn/problems/rotting-oranges/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个二维网格，每个格子可能是空位、鲜橘子或腐烂橘子。每过一分钟，所有与腐烂橘子上下左右相邻的鲜橘子都会变成腐烂橘子，请计算让所有橘子都腐烂所需的最少分钟数。

    输入是网格，输出是最少分钟数；如果最终仍有鲜橘子无法被感染，则返回 `-1`。题目实际要求的是多源同时扩散后的最短时间。

这题不是单源最短路，而是多源同时扩散的层序 BFS。把所有初始腐烂橘子一起入队，队列每向外扩一层就代表过了一分钟；如果最后仍有新鲜橘子没被感染，就说明无法全部腐烂。

???+ solution "994. 腐烂的橘子"

    ```cpp
    --8<-- "code/leetcode/hot100/L994.cpp"
    ```

## 拓扑排序

### 207. 课程表

!!! problem "[207. 课程表](https://leetcode.cn/problems/course-schedule/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定课程总数 `numCourses` 和先修关系数组 `prerequisites`，每个关系表示某门课程必须先修完另一门课程。请判断是否可以修完所有课程。

    输入描述的是一组有向先修约束，输出是布尔值。若依赖关系中存在环，就意味着无法找到满足所有约束的修课顺序。

先修关系天然是一张有向图。若图中有环，就不存在合法的修课顺序；若能把所有入度为零的点逐步删掉，说明整个图可以被拓扑排序，也就意味着所有课程都能修完。

???+ solution "207. 课程表"

    ```cpp
    --8<-- "code/leetcode/hot100/L207.cpp"
    ```

## 前缀树

### 208. 实现 Trie

!!! problem "[208. 实现 Trie (前缀树)](https://leetcode.cn/problems/implement-trie-prefix-tree/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    设计并实现一个 Trie（前缀树），支持 `insert`、`search` 和 `startsWith` 三个操作。`insert` 负责插入单词，`search` 负责判断完整单词是否存在，`startsWith` 负责判断是否存在以给定前缀开头的单词。

    输入是一系列字符串操作，输出需要按查询操作返回对应结果。Trie 的状态需要区分“路径存在”和“完整单词结束”两种情况，因此节点上通常要额外记录结束标记。

Trie 的状态转移是按字符逐层展开的。每个节点最多有 26 个子节点，因此 `insert`、`search` 和 `startsWith` 都只需要沿字符串长度走一遍，复杂度与单词长度成正比。

???+ solution "208. 实现 Trie"

    ```cpp
    --8<-- "code/leetcode/hot100/L208.cpp"
    ```

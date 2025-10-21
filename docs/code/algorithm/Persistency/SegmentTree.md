---
title: 可持久化线段树
tags:
  - Persistent Segment Tree
---

# 可持久化线段树

可持久化线段树是一种支持版本管理的线段树数据结构，允许在进行更新操作时，保留之前的版本，从而可以随时访问和查询历史版本的数据。每次更新操作都会创建一个新的版本，而旧版本的数据仍然可以被访问。

## 可持久化数组

维护这样的一个长度为 $N$ 的数组，支持如下两种操作：

1. 在某个历史版本上修改某一个位置上的值。
2. 访问某个历史版本上的某一位置的值。

此外，每进行一次操作，就会生成一个新的版本。版本编号即为当前操作的编号（从 $1$ 开始编号，版本 $0$ 表示初始状态数组）。

可持久化数组可以用于实现可持久化并查集等数据结构。

???+ note "[【模板】可持久化线段树 1（可持久化数组）](https://www.luogu.com.cn/problem/P3919){target=_blank}"

    ```cpp
    --8<-- "code/Persistency/SegmentTree/P3919.cpp"
    ```

## 可持久化权值线段树

可持久化权值线段树可以用于解决区间第 $k$ 小数、区间元素出现次数等问题。

???+ note "[【模板】可持久化线段树 2](https://www.luogu.com.cn/problem/P3834){target=_blank}"

    !!! hint

        1. 通过离散化将元素值映射到一个较小的范围内，方便在线段树中进行处理。
        2. 构建一个初始版本的线段树，表示空数组。
        3. 对于每个元素，基于前一个版本的线段树，更新该元素所在位置的值，生成一个新的版本。
        4. 查询时，利用两个版本的线段树（区间的左右端点对应的版本）进行差分，计算出区间内每个子区间的元素数量，从而定位第 $k$ 小的元素。

    ```cpp
    --8<-- "code/Persistency/SegmentTree/P3834.cpp"
    ```

??? note "[Buratsuta 3](https://codeforces.com/contest/2149/problem/G){target=_blank}"

    查询区间内所有出现次数超过三分之一（$\lfloor \frac{r - l + 1}{3} \rfloor$）的元素。

    ```cpp
    --8<-- "code/Persistency/SegmentTree/CF2149G.cpp"
    ```

## 标记永久化

标记永久化通过将更新操作的标记永久化，可以避免在查询时进行复杂的标记传播，从而提高查询效率。

??? note "[【模板】线段树 1](https://www.luogu.com.cn/problem/P3372){target=_blank}"

    ```cpp
    --8<-- "code/Persistency/SegmentTree/P3372.cpp"
    ```

## 范围修改可持久

范围修改可持久化线段树是一种结合了可持久化和范围更新的线段树数据结构，允许在进行范围更新操作时，保留之前的版本，从而可以随时访问和查询历史版本的数据。每次更新操作都会创建一个新的版本，而旧版本的数据仍然可以被访问。

??? note "[TTM - To the moon](https://www.spoj.com/problems/TTM/){target=_blank}"

    === "克隆标记"

        ```cpp
        --8<-- "code/Persistency/SegmentTree/TTM_1.cpp"
        ```

    === "标记永久化"

        !!! tip

            通过标记永久化可以减少范围修改可持久线段树的占用空间，因为每次更新只需要克隆受影响的节点，而不需要克隆整个路径上的所有节点，从而节省了空间。

        ```cpp
        --8<-- "code/Persistency/SegmentTree/TTM_2.cpp"
        ```
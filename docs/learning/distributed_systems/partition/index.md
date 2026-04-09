---
title: "分片与路由（Partitioning and Routing）"
tags:
  - 分布式系统
  - 分片
  - 数据模型
---

# 分片与路由（Partitioning and Routing）

对于海量的数据集以及吞吐量极高的场景，单机的存储瓶颈成为了阻碍。分片（Partitions 或者 Sharding）是指将数据均匀切分在不同物理节点以进行水平扩展（Scale Out）的行为。

## 数据分区策略

- 键范围分区。
- 哈希散列分区。

## 核心机制

分片架构中，最常遇到的核心工程挑战在于资源的寻址、负载均衡以及动态扩缩容情况下的稳健迁移：

- [一致性哈希 (Consistent Hashing)](ConsistentHashing.md)：解决节点数目动态变更时的局部路由失效问题，广泛应用于分布式缓存及存储节点寻流。
- [集群扩缩容与数据重平衡 (Rebalancing)](Rebalancing.md)：讨论纯缓存与持久化存储架构下的不同迁移流派，并分析幽灵复活与各种一致性并发补偿策略（LWW、墓碑与 WAL）。

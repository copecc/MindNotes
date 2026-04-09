---
title: 排序算法
tags:
  - 排序
  - Sort
  - 基数排序
  - Radix Sort
  - 败者树
---

# 排序算法

## 基数排序

基数排序（$\text{Radix Sort}$）将待排序的元素拆分为 $k$ 个关键字，逐一对各个关键字排序后完成对所有元素的排序。

???+ note "基数排序"
    基数排序适用于整数排序，尤其是当整数的范围较大而数量较少时。  
    时间复杂度 $O(d \cdot (n + k))$，空间复杂度 $O(n + k)$。

    ```cpp
    void radix_sort(vector<int> &arr, int base = 10) {
      int n       = arr.size();
      int min_val = *min_element(arr.begin(), arr.end());
      // 先将所有数减去最小值，变为非负数, 因为基数排序只能处理非负数
      for_each(arr.begin(), arr.end(), [&](int &x) { x -= min_val; });
      int max_val   = *max_element(arr.begin(), arr.end());
      int max_digit = 0;  // 最大位数
      while (max_val != 0) {
        max_digit++;
        max_val /= base;
      }

      // 基数排序
      vector<int> output(n);
      vector<int> count(base, 0);
      for (int offset = 1; max_digit > 0; offset *= base, max_digit--) {
        fill(count.begin(), count.end(), 0);
        // 统计每个桶的元素个数
        for (int i = 0; i < n; i++) { count[(arr[i] / offset) % base]++; }
        // 计算每个桶的结束位置
        for (int i = 1; i < base; i++) { count[i] += count[i - 1]; }
        for (int i = n - 1; i >= 0; i--) {
          int &pos        = count[(arr[i] / offset) % base];  // 获取桶的结束位置
          output[pos - 1] = arr[i];                           // 放入对应桶中
          pos--;                                              // 该桶结束位置前移
        }
        arr = output;
      }
      // 将所有数加上最小值，恢复原值
      for_each(arr.begin(), arr.end(), [&](int &x) { x += min_val; });
    }
    ```

## 败者树

败者树（Loser Tree）是一种特殊的树形数据结构，本质上是一棵完全二叉树。它是锦标赛排序（Tournament Sort）的一种优化变体，主要用于高效地实现多路归并（Multi-way Merge）。

可以把它想象成一场淘汰赛，只不过这场比赛的记录方式有些特别：各个节点记录的是败者，而胜者则继续晋级，参与更高层级的比赛。（注：在排序问题中，如果要求升序排列，那么值较小的元素就是胜者，值较大的元素就是败者）。

### 败者树的核心机制

一棵用于 $k$ 路归并的败者树包含 $k$ 个叶子节点和 $k-1$ 个内部节点：

- 叶子节点：存放 $k$ 个数据流当前参与比较的元素。
- 内部节点：记录在这场比赛中失败者的索引（即哪个数据流被打败了）。
- 冠军节点：在树的最高层（通常在根节点上方设一个额外的节点，如 ls[0]），记录最终的绝对胜者（全局最小值）。

运行过程如下：

- 初始化：从 $k$ 个数据流中各取第一个元素放入叶子节点，两两比赛。胜者向上晋级，败者的索引留在当前的内部节点。最终冠军诞生并输出。
- 更新：冠军输出后，冠军所在的数据流会补充一个新元素进入叶子节点。此时，新元素只需要沿着它到根节点的路径，依次与各个内部节点记录的历史败者进行比较。
    - 如果新元素比内部节点记录的败者小（新元素胜），新元素继续向上晋级，原败者留在该节点。
    - 如果新元素比内部节点记录的败者大（新元素败），新元素留在该节点成为新的败者，原记录的败者作为胜者向上晋级。

在传统的胜者树中，内部节点记录的是胜者。当冠军被取走，新元素加入时，新元素必须与它的兄弟节点进行比较。为了找到兄弟节点，程序需要访问父节点，再通过父节点找到兄弟节点，这增加了内存访问的开销。

而败者树直接在父节点就记录了上次比赛的对手（败者），新元素只需要直接和父节点记录的值比较即可，无需寻找兄弟节点。这使得代码实现更简单，访存效率更高，尤其在处理海量数据时性能优势明显。

与胜者树相比，败者树在更新时只需要比较 $O(\log k)$ 次，而胜者树可能需要比较 $O(k)$ 次，因此在多路归并中，败者树的效率更高。


### 主要应用场景

败者树最核心、几乎也是唯一的应用场景就是多路归并排序，特别是在以下特定领域：

- 外部排序（External Sorting）：当需要排序的数据量极大（例如几十GB甚至TB级别），内存无法一次性装下时，数据库和操作系统会采用外部排序。
    1. 先将大文件切分成多个可以装入内存的小块。
    2. 在内存中分别对这些小块进行排序，生成多个有序的临时文件（称为归并段）。
    3. 使用 败者树 对这 $k$ 个有序文件进行多路归并，合并成一个最终的大文件。每次选出最小值只需要 $O(\log k)$ 的比较次数。
- 数据库系统的查询引擎：在执行 ORDER BY 或多表 JOIN 操作时，如果涉及的数据量超过了内存限制的 Buffer Pool，数据库引擎（如 MySQL, PostgreSQL）在底层会利用外部排序算法，其中归并阶段就会使用败者树来合并中间结果。
- 海量日志处理与大数据框架：在 Hadoop (MapReduce) 或 Spark 等分布式计算框架的 Shuffle 阶段，节点需要将大量已局部排序的数据流合并成全局有序的数据流，败者树同样是底层常用的合并数据结构。

???+ note "[23. 合并 K 个升序链表](https://leetcode.cn/problems/merge-k-sorted-lists/){target=_blank}"

    给你一个链表数组，每个链表都已经按升序排列。

    请你将所有链表合并到一个升序链表中，返回合并后的链表。

    === "败者树"

        时间复杂度 $O(n \log k)$，空间复杂度 $O(k)$。

        ```cpp
        class Solution {
        private:
          int k;
          vector<int> tree;          // 存储败者所在的链表索引
          vector<ListNode *> nodes;  // 存储当前各个链表的头节点

          // 获取给定链表索引的当前节点值
          int getValue(int idx) {
            if (idx == -1) { // 哨兵节点：用于初始化时使得第一个到达内部节点的真实元素必定落败并被记录
              return INT_MIN;
            }
            if (nodes[idx] == nullptr) { // 链表耗尽：视为无穷大，确保在求最小值的比较中必定落败
              return INT_MAX;
            }
            return nodes[idx]->val;
          }

          // 调整败者树：s 为刚刚更新数据所在的链表索引
          void adjust(int s) {
            int t = (s + k) / 2;  // 计算对应的父节点（内部节点）索引

            while (t > 0) {
              // 如果当前元素的值大于内部节点记录的值，则当前元素落败
              if (getValue(s) > getValue(tree[t])) {
                // 交换：将当前败者留在内部节点 tree[t]，胜者继续向上比较
                int temp = s;
                s        = tree[t];
                tree[t]  = temp;
              }
              t /= 2;  // 移动到上一层父节点
            }
            tree[0] = s; // tree[0] 记录最终到达顶部的全局最小元素（胜者）的索引
          }

        public:
          ListNode *mergeKLists(vector<ListNode *> &lists) {
            k = lists.size();
            if (k == 0) return nullptr;
            if (k == 1) return lists[0];

            nodes = lists;
            tree.assign(k, -1);  // 初始化所有内部节点为哨兵索引 -1
            // 初始化败者树：从后向前依次将所有初始节点推入树中进行调整
            for (int i = k - 1; i >= 0; --i) { adjust(i); }

            ListNode dummy(0);
            ListNode *tail = &dummy;

            // 持续提取胜者并更新败者树
            while (true) {
              int winnerIdx = tree[0];
              // 如果当前胜者为空，说明所有链表均已耗尽（最小值为 INT_MAX）
              if (nodes[winnerIdx] == nullptr) { break; }
              // 将胜者节点接入结果链表
              tail->next = nodes[winnerIdx];
              tail       = tail->next;
              // 胜者所在链表的指针后移，指向下一个元素
              nodes[winnerIdx] = nodes[winnerIdx]->next;
              // 从刚刚提取了元素的链表位置重新调整败者树
              adjust(winnerIdx);
            }

            return dummy.next;
          }
        };
        ```

    === "最小堆"

        时间复杂度 $O(n \log k)$，空间复杂度 $O(k)$。

        ```cpp
        class Solution {
        public:
          ListNode *mergeKLists(vector<ListNode *> &lists) {
            auto cmp = [](const ListNode *a, const ListNode *b) { return a->val > b->val; };
            priority_queue<ListNode *, vector<ListNode *>, decltype(cmp)> pq;
            for (auto head : lists) {
              if (head) { pq.push(head); }
            }

            ListNode dummy{};  // 哨兵节点，作为合并后链表头节点的前一个节点
            auto cur = &dummy;
            while (!pq.empty()) {
              auto node = pq.top();
              pq.pop();
              if (node->next) { pq.push(node->next); }
              cur->next = node;       // 把 node 添加到新链表的末尾
              cur       = cur->next;  // 准备合并下一个节点
            }
            return dummy.next;  // 哨兵节点的下一个节点就是新链表的头节点
          }
        };
        ```


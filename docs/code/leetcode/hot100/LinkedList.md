---
title: 链表
tags:
  - hot100
  - 链表
---

# 链表

链表题的共同点是只修改指针关系，不依赖随机访问。常见手段包括快慢指针、哨兵节点、局部反转、分治归并和哈希表映射。

## 反转与快慢指针

### 206. 反转链表

!!! problem "[206. 反转链表](https://leetcode.cn/problems/reverse-linked-list/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定单链表的头节点 `head`，请将链表中的节点顺序原地反转，并返回反转后的头节点。反转过程中只能重新调整指针指向，不能额外构造一条新的链表来替代原链表。

    输入只提供头节点，输出是反转后链表的头节点；节点值本身不需要改变，关键在于把每个节点的 `next` 指针重新连接到前驱节点上。

这题的核心是不变量：`prev` 始终指向已经反转好的前缀，`cur` 指向尚未处理的后缀。每轮把 `cur->next` 改向 `prev`，再同步前移两个指针，就能把整条链表线性翻转。

???+ solution "206. 反转链表"

    ```cpp
    --8<-- "code/leetcode/hot100/L206.cpp"
    ```

### 141. 环形链表

!!! problem "[141. 环形链表](https://leetcode.cn/problems/linked-list-cycle/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定链表头节点 `head`，判断链表中是否存在环。所谓有环，是指某个节点的 `next` 指针最终会回到链表中已经访问过的节点，而不是指向空指针。

    函数需要返回一个布尔值：若链表中存在环则返回 `true`，否则返回 `false`。题目只关心是否成环，不要求返回入口位置。

快慢指针的关键不是“跑得快”，而是利用相对速度判断是否进入同一位置。若存在环，快指针会在环内逐步追上慢指针；若不存在环，快指针会先走到空指针。

???+ solution "141. 环形链表"

    ```cpp
    --8<-- "code/leetcode/hot100/L141.cpp"
    ```

### 142. 环形链表 II

!!! problem "[142. 环形链表 II](https://leetcode.cn/problems/linked-list-cycle-ii/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定链表头节点 `head`，若链表中存在环，请返回环的入口节点；如果不存在环，则返回空节点。这里的入口节点是指从头节点出发第一次进入环时遇到的那个节点。

    题目要求直接返回节点引用本身，而不是节点值。若链表无环，答案应当明确表示为空。

先让快慢指针相遇，再把其中一个指针移回头结点，随后两个指针同步前进。因为从头节点到入口的距离，恰好等于相遇点到入口沿环绕回去的距离，所以第二次相遇的位置就是入口。

???+ solution "142. 环形链表 II"

    ```cpp
    --8<-- "code/leetcode/hot100/L142.cpp"
    ```

### 234. 回文链表

!!! problem "[234. 回文链表](https://leetcode.cn/problems/palindrome-linked-list/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定单链表的头节点 `head`，判断链表节点值从前往后读和从后往前读是否一致。若一致，则该链表是回文链表；否则不是。

    输入只包含链表头节点，输出是布尔值。题目关注的是值序列是否对称，不要求修改链表内容，也不要求返回任何额外结构。

这题通常拆成三步：找中点、反转后半段、逐节点比较。这样就把“逆序访问后半段”转成了原地反转问题，仍然保持线性时间和常量额外空间。

???+ solution "234. 回文链表"

    ```cpp
    --8<-- "code/leetcode/hot100/L234.cpp"
    ```

## 基础操作

### 21. 合并两个有序链表

!!! problem "[21. 合并两个有序链表](https://leetcode.cn/problems/merge-two-sorted-lists/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定两个升序链表的头节点 `list1` 和 `list2`，请将它们合并成一个新的升序链表并返回。合并后链表中的节点应当按非降序排列，且所有原有节点都应被保留在结果中。

    输入是两条已经按升序排列的单链表，输出是合并后的链表头节点。题目不要求创建新的值节点，通常直接通过重连原节点完成。

这是典型的双指针归并。维护一个哨兵节点和尾指针，每次从两个表头中取更小的那个接到结果后面即可，剩余部分直接整体拼接。

???+ solution "21. 合并两个有序链表"

    ```cpp
    --8<-- "code/leetcode/hot100/L21.cpp"
    ```

### 2. 两数相加

!!! problem "[2. 两数相加](https://leetcode.cn/problems/add-two-numbers/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定两条非空链表 `l1` 和 `l2`，它们分别表示两个非负整数，数字按逆序存储在链表节点中。请把这两个整数相加，并以同样的逆序链表形式返回结果。

    每个节点只保存一位数字，结果链表中的每个节点也只能表示一位。若最高位相加后仍有进位，需要在结果链表末尾继续补出新节点。

核心是逐位模拟竖式加法，同时维护进位 `carry`。只要任一链表还有节点或者进位不为零，就继续向后生成新节点。

???+ solution "2. 两数相加"

    ```cpp
    --8<-- "code/leetcode/hot100/L2.cpp"
    ```

### 19. 删除链表的倒数第 N 个结点

!!! problem "[19. 删除链表的倒数第 N 个结点](https://leetcode.cn/problems/remove-nth-node-from-end-of-list/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定链表头节点 `head` 和整数 `n`，请删除链表中的倒数第 `n` 个节点，并返回删除后的链表头节点。这里的“倒数第 `n` 个”指从链表尾部向前数第 `n` 个节点。

    题目保证 `n` 有效，因此一定能定位到要删除的节点。输出应当是完成删除操作后的链表结构，链表其余部分保持原有相对顺序。

快慢指针把“倒数第 `n` 个”转成了“正数第 `len-n` 个”。先让快指针领先 `n` 步，再让两个指针同步移动，慢指针就会停在目标节点的前驱位置。

???+ solution "19. 删除链表的倒数第 N 个结点"

    ```cpp
    --8<-- "code/leetcode/hot100/L19.cpp"
    ```

### 24. 两两交换链表中的节点

!!! problem "[24. 两两交换链表中的节点](https://leetcode.cn/problems/swap-nodes-in-pairs/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定单链表的头节点 `head`，请按相邻两个节点为一组交换它们的位置，并返回交换后的链表头节点。若链表长度为奇数，最后一个节点保持原位。

    题目要求交换的是节点位置，不是节点值；因此需要重新连接指针关系，使每一对相邻节点在结果链表中的顺序互换。

这题只需要在局部维护四个指针：前驱、第一节点、第二节点和后续剩余部分。每轮完成一次局部重连后，把前驱移动到交换后的尾部，继续处理下一对。

???+ solution "24. 两两交换链表中的节点"

    ```cpp
    --8<-- "code/leetcode/hot100/L24.cpp"
    ```

### 25. K 个一组翻转链表

!!! problem "[25. K 个一组翻转链表](https://leetcode.cn/problems/reverse-nodes-in-k-group/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定链表头节点 `head` 和整数 `k`，请将链表中的节点每 `k` 个一组进行翻转，并返回处理后的链表。对于不足 `k` 个节点的最后一段，保持原样不翻转。

    输入中的每一组都必须先确认节点数足够，再对这一段做原地翻转；题目关注的是链表结构的重排，而不是节点值的修改。

先确认当前段是否至少有 `k` 个节点；如果有，就把这一段原地翻转，再递归/迭代处理后续链表。这个问题的难点不在翻转本身，而在分段边界必须严格对齐。

???+ solution "25. K 个一组翻转链表"

    ```cpp
    --8<-- "code/leetcode/hot100/L25.cpp"
    ```

## 进阶结构

### 160. 相交链表

!!! problem "[160. 相交链表](https://leetcode.cn/problems/intersection-of-two-linked-lists/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定两条单链表的头节点 `headA` 和 `headB`，请找出它们相交的起始节点。如果两条链表没有交点，则返回空节点。

    所谓相交，指两条链表在某个节点之后共享同一段后缀，后续节点引用完全相同；题目要求返回这个共享部分的第一个节点，而不是值相同的节点。

双指针分别遍历两条链表，走到末尾后切换到另一条链表的头部。这样两个指针最终走过的路径长度相同，如果存在交点，就会在同一节点同步相遇。

???+ solution "160. 相交链表"

    ```cpp
    --8<-- "code/leetcode/hot100/L160.cpp"
    ```

### 138. 随机链表的复制

!!! problem "[138. 随机链表的复制](https://leetcode.cn/problems/copy-list-with-random-pointer/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一条带有 `next` 和 `random` 指针的链表头节点，请返回这条链表的深拷贝。新链表中的每个节点都应是全新的对象，且 `next` 和 `random` 的指向关系要与原链表一致。

    `random` 指针可以指向链表中的任意节点，也可以指向空节点；输出需要保留原链表的结构语义，但不能复用原节点对象。

`next` 和 `random` 都可能指向任意旧节点，所以必须把“旧节点到新节点”的映射保存下来。用哈希表先建点、再连边，是最直接也最稳定的写法。

???+ solution "138. 随机链表的复制"

    ```cpp
    --8<-- "code/leetcode/hot100/L138.cpp"
    ```

### 148. 排序链表

!!! problem "[148. 排序链表](https://leetcode.cn/problems/sort-list/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定链表头节点 `head`，请对链表中的节点按升序排序，并返回排序后的头节点。题目要求时间复杂度优于 $O(n^2)$，因此不能使用简单的冒泡或插入排序直接解决。

    输入是单链表，输出是排序后链表的头节点；节点值可以重排，但通常仍通过重连原节点完成排序。

链表不适合随机访问，所以更自然的做法是归并排序：先快慢指针拆成两半，再分别排序，最后把两个有序链表合并。这个分治结构正好匹配链表的线性访问特性。

???+ solution "148. 排序链表"

    ```cpp
    --8<-- "code/leetcode/hot100/L148.cpp"
    ```

### 23. 合并 K 个升序链表

!!! problem "[23. 合并 K 个升序链表](https://leetcode.cn/problems/merge-k-sorted-lists/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个包含 `k` 条升序链表的数组，请将所有链表合并成一条升序链表并返回。合并后的结果应保持整体非降序，且每条输入链表中的节点都应出现在结果中。

    输入是若干条已经排好序的单链表，输出是合并后的链表头节点。题目本质上是把两路归并推广到多路归并。

本质上是把“两个有序表归并”推广到 `k` 路归并。最常见的实现是小根堆：堆顶始终是当前所有头节点中的最小值，弹出后再把它的后继补回去。

???+ solution "23. 合并 K 个升序链表"

    ```cpp
    --8<-- "code/leetcode/hot100/L23.cpp"
    ```

### 146. LRU 缓存

!!! problem "[146. LRU 缓存](https://leetcode.cn/problems/lru-cache/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    设计一个支持 `get` 和 `put` 的缓存结构 `LRUCache`。当缓存容量达到上限时，需要淘汰最近最少使用的键值对；如果某个键被访问或更新，它的使用状态要同步刷新。

    `get` 需要返回指定键对应的值，不存在时返回 `-1`；`put` 需要插入或更新键值对，并在必要时触发淘汰。题目要求两个操作都尽量高效完成。

这题的关键是把“访问顺序”显式维护出来。`list` 负责保存使用顺序，哈希表负责把键直接定位到链表节点；每次访问都把节点移动到表头，淘汰时删掉表尾即可。

???+ solution "146. LRU 缓存"

    ```cpp
    --8<-- "code/leetcode/hot100/L146.cpp"
    ```

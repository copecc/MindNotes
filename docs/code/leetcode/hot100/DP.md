---
title: 动态规划
tags:
  - hot100
  - 动态规划
---

# 动态规划

动态规划题的共同点是：把原问题拆成若干个状态，并且每个状态都能从更小的状态转移而来。Hot100 里的 DP 题通常会落到四种模式：线性递推、序列 DP、背包 DP、以及把字符串前缀当作状态的划分型 DP。

## 爬楼梯

!!! problem "[70. 爬楼梯](https://leetcode.cn/problems/climbing-stairs/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    假设你正在爬楼梯，楼梯一共有 `n` 阶。每次你可以爬 `1` 阶或者 `2` 阶，且允许在中途停留在任意阶梯上。请计算爬到楼顶一共有多少种不同的方法。

    这里的“不同方法”指台阶选择序列不同就算不同方案。输入是一个正整数 `n`，输出是方案总数。

这是最基础的线性递推。

设 `f(n)` 表示爬到第 `n` 阶的方法数。最后一步只有两种可能：

- 从第 `n - 1` 阶走 `1` 步过来。
- 从第 `n - 2` 阶走 `2` 步过来。

因此有：

$$
f(n) = f(n - 1) + f(n - 2)
$$

边界是 `f(1) = 1`、`f(2) = 2`。这个递推和 Fibonacci 序列完全一致，只需要用两个变量滚动保存即可。

???+ solution "70. 爬楼梯"

    ```cpp
    --8<-- "code/leetcode/hot100/L70.cpp"
    ```

## 杨辉三角

!!! problem "[118. 杨辉三角](https://leetcode.cn/problems/pascals-triangle/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个非负整数 `numRows`，请生成杨辉三角的前 `numRows` 行并返回。第 `1` 行只有一个元素 `1`，每一行的长度都比上一行多 `1`。

    输出需要按二维列表的形式组织，满足杨辉三角的定义：每个位置等于上一行相邻两个位置之和，行首和行尾都为 `1`。

杨辉三角的第 `i` 行第 `j` 个位置，只依赖上一行的两个相邻位置。

如果把第 `i` 行记作 `triangle[i]`，那么：

- 每行的首尾都是 `1`。
- 中间位置满足：

$$
triangle[i][j] = triangle[i - 1][j - 1] + triangle[i - 1][j]
$$

这说明整张表可以逐行构造。每生成一行时，只需要上一行作为状态来源，不需要回头修改更早的行。

???+ solution "118. 杨辉三角"

    ```cpp
    --8<-- "code/leetcode/hot100/L118.cpp"
    ```

## 打家劫舍

!!! problem "[198. 打家劫舍](https://leetcode.cn/problems/house-robber/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个整数数组 `nums`，其中 `nums[i]` 表示第 `i` 间房屋藏有的金额。你不能同时偷相邻的两间房屋，否则会触发警报。请返回在不触发警报的前提下，能够偷到的最大金额。

    输入数组长度可以为 `1`，每个元素都是非负整数。答案是一个整数，表示可获得的最大收益。

设 `dp[i]` 表示考虑到第 `i` 间房屋时的最大收益。对第 `i` 间房屋只有两种选择：

- 不偷它，那么收益就是 `dp[i - 1]`。
- 偷它，那么前一间就不能偷，收益是 `dp[i - 2] + nums[i]`。

于是转移方程是：

$$
dp[i] = \max(dp[i - 1],\ dp[i - 2] + nums[i])
$$

这个状态只依赖前两项，所以可以直接压成两个滚动变量。

???+ solution "198. 打家劫舍"

    ```cpp
    --8<-- "code/leetcode/hot100/L198.cpp"
    ```

## 完全平方数

!!! problem "[279. 完全平方数](https://leetcode.cn/problems/perfect-squares/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个正整数 `n`，请找出若干个完全平方数，使它们的和等于 `n`，并返回所需的最少数量。这里的完全平方数包括 `1, 4, 9, 16, ...`。

    你可以重复使用同一个平方数，目标是让使用的平方数个数尽可能少。输入只有一个整数 `n`，输出最少个数。

这题可以看成完全背包：每个完全平方数都可以使用无限次，目标是用最少的物品数凑出容量 `n`。

设 `dp[j]` 表示凑出和 `j` 所需的最少平方数个数。对于每个平方数 `sq`，从小到大更新：

$$
dp[j] = \min(dp[j],\ dp[j - sq] + 1)
$$

因为平方数可以重复使用，所以内层循环必须正向遍历，这样同一个平方数才能在同一轮里被多次选取。

???+ solution "279. 完全平方数"

    ```cpp
    --8<-- "code/leetcode/hot100/L279.cpp"
    ```

## 零钱兑换

!!! problem "[322. 零钱兑换](https://leetcode.cn/problems/coin-change/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给你一个整数数组 `coins`，表示若干种不同面值的硬币，以及一个整数 `amount`，表示目标金额。每种硬币都可以无限次使用，请计算凑出 `amount` 所需的最少硬币数。

    如果无法凑出目标金额，返回 `-1`。输入数组中的面值均为正整数，答案表示最少硬币数量。

这也是完全背包，只是状态从“平方数”换成了“硬币面值”。

设 `dp[j]` 表示凑出金额 `j` 的最少硬币数。对每个硬币 `coin`，更新：

$$
dp[j] = \min(dp[j],\ dp[j - coin] + 1)
$$

初始化时 `dp[0] = 0`，其余状态设为不可达。最后如果 `dp[amount]` 仍然不可达，就返回 `-1`。

这个问题和 `279` 的区别只在于物品含义不同，状态和转移完全一致。

???+ solution "322. 零钱兑换"

    ```cpp
    --8<-- "code/leetcode/hot100/L322.cpp"
    ```

## 单词拆分

!!! problem "[139. 单词拆分](https://leetcode.cn/problems/word-break/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给你一个字符串 `s` 和一个单词字典 `wordDict`，判断 `s` 能否被拆分成若干个字典中的单词，且拆分后的每一段都必须完整出现在字典中。

    字典中的单词可以重复使用，切分得到的各段需要按原顺序拼接回整个字符串。返回值是一个布尔值，表示是否存在这样的拆分方式。

这题的状态是前缀。设 `dp[i]` 表示前 `i` 个字符能否被成功拆分。

如果存在一个切分点 `j`，满足：

- `dp[j] = true`
- `s[j:i]` 在字典中

那么 `dp[i]` 就可以置为 `true`。也就是说，当前前缀能否成立，只取决于更短前缀是否成立，以及最后一段是否在字典里。

这个模型的本质是把字符串拆分问题转成前缀可达性问题。

???+ solution "139. 单词拆分"

    ```cpp
    --8<-- "code/leetcode/hot100/L139.cpp"
    ```

## 乘积最大子数组

!!! problem "[152. 乘积最大子数组](https://leetcode.cn/problems/maximum-product-subarray/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给你一个整数数组 `nums`，请在其中选择一个连续子数组，使得该子数组元素乘积最大，并返回这个最大乘积。

    子数组至少包含一个元素，数组中可能包含负数和 `0`，因此乘积在遍历过程中可能发生符号翻转。

这题和最大子段和不同，因为负数会翻转乘积的大小关系。仅维护一个最大值不够，还必须同时维护当前最小值。

设：

- `mx` 表示以当前位置结尾的最大乘积。
- `mn` 表示以当前位置结尾的最小乘积。

当读入一个负数时，最大值和最小值会互换角色，所以需要先交换 `mx` 和 `mn`，再更新：

$$
mx = \max(x,\ mx \cdot x)
$$

$$
mn = \min(x,\ mn \cdot x)
$$

`mx` 维护的是当前最优答案，`mn` 维护的是后续可能翻正的最差状态。

???+ solution "152. 乘积最大子数组"

    ```cpp
    --8<-- "code/leetcode/hot100/L152.cpp"
    ```

## 最长递增子序列

!!! problem "[300. 最长递增子序列](https://leetcode.cn/problems/longest-increasing-subsequence/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给你一个整数数组 `nums`，请找出其中最长严格递增子序列的长度。子序列不要求连续，但必须保持原有相对顺序。

    返回值是这个最长子序列的元素个数，而不是子序列本身。

这题有一个经典的 DP 优化写法。维护数组 `tails`，其中 `tails[k]` 表示长度为 `k + 1` 的递增子序列所能取得的最小结尾值。

当遍历到一个新数 `x` 时：

- 如果 `x` 比所有结尾都大，就把它接到最后，形成更长的子序列。
- 否则在 `tails` 中找到第一个大于等于 `x` 的位置，用 `x` 替换它。

这样做不会改变当前长度的可行性，却会把对应长度的结尾尽量压小，从而给后续数字留下更大的延展空间。

它等价于把“长度”当作状态，把“最小结尾”当作状态压缩后的表示。

???+ solution "300. 最长递增子序列"

    ```cpp
    --8<-- "code/leetcode/hot100/L300.cpp"
    ```

## 分割等和子集

!!! problem "[416. 分割等和子集](https://leetcode.cn/problems/partition-equal-subset-sum/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给你一个只包含正整数的非空数组 `nums`，请判断是否可以把它分成两个子集，使两个子集中的元素和相等。

    题目要求返回一个布尔值。若总和为奇数，则一定无法划分；否则需要判断是否存在一个子集，其元素和恰好等于总和的一半。

如果总和是奇数，答案一定为 `false`。否则问题就变成：能否从数组中选出一些数，使它们的和恰好等于总和的一半。

这就是一个 0/1 背包可达性问题。设 `dp[j]` 表示是否能凑出和 `j`，对每个数字 `num`，都要从大到小更新：

$$
dp[j] = dp[j] \lor dp[j - num]
$$

倒序遍历是关键，因为每个数字只能使用一次。如果正序更新，就会在同一轮里重复使用当前数字，状态语义会被破坏。

???+ solution "416. 分割等和子集"

    ```cpp
    --8<-- "code/leetcode/hot100/L416.cpp"
    ```

## 最长有效括号

!!! problem "[32. 最长有效括号](https://leetcode.cn/problems/longest-valid-parentheses/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个只包含 `'('` 和 `')'` 的字符串 `s`，请返回其中最长有效括号子串的长度。有效括号子串要求左右括号能够正确配对，并且子串本身是连续的。

    如果不存在有效括号子串，返回 `0`。输入是一个字符串，输出是最长合法连续片段的长度。

这题既有栈写法，也有很标准的动态规划写法。若按动态规划来理解，可以令 `dp[i]` 表示“以位置 `i` 结尾的最长有效括号长度”。这样状态天然带有“结尾必须固定在当前位置”的约束，便于做转移。

只有当 `s[i] == ')'` 时，`dp[i]` 才可能大于 `0`。分两种情况：

- 如果 `s[i - 1] == '('`，那么当前位置和前一个字符刚好构成一对 `()`，于是：

$$
dp[i] = dp[i - 2] + 2
$$

- 如果 `s[i - 1] == ')'`，那么需要先看前一个位置已经形成的有效括号长度 `dp[i - 1]`，再尝试去匹配它左侧紧邻的那个字符。若该字符是 `'('`，就能把两段接起来：

$$
dp[i] = dp[i - 1] + 2 + dp[i - dp[i - 1] - 2]
$$

最后一项的作用是把更左侧已经存在的有效括号段一并接上。因此这题虽然表面是括号匹配，但按状态定义来看，属于典型的“一维前缀结尾 DP”。

???+ solution "32. 最长有效括号"

    ```cpp
    --8<-- "code/leetcode/hot100/L32.cpp"
    ```

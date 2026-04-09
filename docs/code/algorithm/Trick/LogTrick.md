---
title: Log Trick
tags:
  - 对数技巧
  - Log Trick
---

# Log Trick

!!! info "参考"

    本文整理自知乎专栏文章：[LogTrick 入门教程](https://zhuanlan.zhihu.com/p/1933215367158830792){target=_blank}。

Log Trick 是一种用于优化特定子数组查询问题的算法技巧。它常用于处理：

- 求所有子数组经过某种运算后的最值求解
- 求所有子数组经过某种运算后等于特定值计数问题

典型运算有按位或 `|`、按位与 `&`、`gcd`。  

其核心原理在于：对于固定右端点的所有子数组，随着左端点的向左扩展，运算结果的种类数具有极严格的对数级别上限。这使得原本暴力枚举子数组需要 $O(n^2)$ 的时间复杂度，能够被优化至 $O(n \log M)$，其中 $M$ 为数组元素的最大值。

## 核心观察

假设存在给定数组 $A$ 以及满足结合律的二元运算 $\operatorname{op}$。固定右端点 $r$，定义子数组的区间运算结果为：

$$f(l, r) = A[l] \operatorname{op} A[l+1] \operatorname{op} \cdots \operatorname{op} A[r]$$

当满足以下三个条件时，该问题适用 Log Trick：

- 可结合性：状态可以通过增量方式更新，即 $f(l, r) = f(l, r-1) \operatorname{op} A[r]$。
- 单调性：随着左端点 $l$ 不断向左扩展，区间运算结果具有单调递增或单调递减的特性。
- 状态收敛性：对于固定的右端点，随着区间长度增加，$f(l, r)$ 发生改变的次数极少，不同结果的种类数量上限为 $O(\log M)$。

??? note "单调性和状态收敛性的关系"

    单调性保证了状态的变化方向一致，而状态收敛性则限制了状态变化的总次数。这两者共同作用，使得 Log Trick 能够高效地处理子数组查询问题。

    特定运算的数据缩减机制：
    
    - 按位或 (OR)：区间扩展时结果单调递增。每一位二进制位最多只能从 $0$ 翻转为 $1$ 一次。因此，不同结果的数量不会超过最大元素的二进制位数，即 $O(\log M)$。
    - 按位与 (AND)：区间扩展时结果单调递减。每一位二进制位最多只能从 $1$ 翻转为 $0$ 一次，同理，不同结果数量上限为 $O(\log M)$。
    - 最大公约数 (GCD)：区间扩展时结果单调递减。如果引入新元素后 GCD 发生实质性变化（即新值严格小于旧值），由于新 GCD 必定是原 GCD 的真因子，其数值至多为原值的一半。因此不同结果的最大数量受限于最大元素的质因子总数，即 $O(\log M)$。
    - 最小公倍数 (LCM)：如果新值严格大于旧值，新值至少是原值的两倍（需注意规避整型溢出）。

## 最值类问题

当题目要求求解“最接近某个值的子数组”或“运算后的极限最值”时，通常采用原地修改结合提前剪枝的实现。

设 $nums[j]$ 维护的是当前右端点固定为 $i$ 时，子数组区间 $[j, i]$ 的运算结果。当引入新元素 $x = nums[i]$ 时，需将所有历史状态与 $x$ 执行运算。

!!! example "最值类通用模板"

    ```cpp
    for (int i = 0; i < n; i++) {
        int x = nums[i];
        update_answer(x); // 长度为 1 的子数组本身
        
        // 从 i-1 向左遍历扩展左端点
        for (int j = i - 1; j >= 0; j--) {
            // 如果引入新元素 x 后，区间 [j, i-1] 的状态未发生改变
            // 意味着继续向左扩展的所有区间状态均不会改变，直接剪枝退出
            if (op(nums[j], x) == nums[j]) {
                break; 
            }
            nums[j] = op(nums[j], x);
            update_answer(nums[j]);
        }
    }
    ```

- 原理解析：由于运算的单调性，一旦出现 $\operatorname{op}(nums[j], x) == nums[j]$，即代表吸收 $x$ 已经无法产生新的位变化或因子变化，后续更长区间的结果也必然保持不变。
- 约束：此方法会覆盖原数组中的数据。若后续流程仍依赖原始数组元素，需在执行前进行数据拷贝。


???+ note "[3171. 找到按位或最接近 K 的子数组](https://leetcode.cn/problems/find-subarray-with-bitwise-or-closest-to-k/){target=_blank}"

    给你一个数组 $nums$ 和一个整数 $k$，找到 $nums$ 的一个子数组，满足子数组中所有元素按位或运算 OR 的值与 $k$ 的 绝对差尽可能小。
    
    即选择一个子数组 $nums[l..r]$ 满足 $|k - (nums[l] OR nums[l + 1] ... OR nums[r])|$ 最小。返回最小的绝对差值。

    ```cpp
    class Solution {
    public:
        int minimumDifference(vector<int>& nums, int k) {
            int ans = INT_MAX;
            for (int i = 0; i < nums.size(); i++) {
                int x = nums[i];
                ans = min(ans, abs(x - k));
                for (int j = i - 1; j >= 0 && (nums[j] | x) != nums[j]; j--) {
                    nums[j] |= x;
                    ans = min(ans, abs(nums[j] - k));
                }
            }
            return ans;
        }
    };
    ```

??? note "[1521. 找到最接近目标值的函数值](https://leetcode.cn/problems/find-a-value-of-a-mysterious-function-closest-to-target/){target=_blank}"

    找到一个子数组，使得该子数组中所有元素的按位与运算 AND 的结果与目标值 $target$ 的绝对差最小。返回这个最小的绝对差。

    ```cpp
    class Solution {
    public:
        int closestToTarget(vector<int>& arr, int target) {
            int ans = INT_MAX;
            for (int i = 0; i < arr.size(); i++) {
                int x = arr[i];
                ans = min(ans, abs(x - target));
                for (int j = i - 1; j >= 0 && (arr[j] & x) != arr[j]; j--) {
                    arr[j] &= x;
                    ans = min(ans, abs(arr[j] - target));
                }
            }
            return ans;
        }
    };
    ```

## 计数类问题

当题目要求统计“结果恰好等于某个值 $k$ 的子数组个数”或“汇总每种运算结果出现的频次”时，无法直接使用提前剪枝。由于数值未发生变化并不意味着区间不存在，具有相同结果的不同左端点均需被计入总数。

此时需要使用状态压缩法。固定右端点 $r$ 时，维护一个有序列表记录 $(运算结果, 出现次数)$。因为即使某个值不再变化，它仍然可能对应很多不同的左端点，这些子数组都需要计入答案。

固定右端点 $r$ 后，维护一个列表表示所有以 $r$ 结尾的子数组，按运算结果分组后的统计。由于不同结果只有 `O(log M)` 个，所以这个列表始终很短。  

$$
\text{cur} = [(value_1, cnt_1), (value_2, cnt_2), \dots]
$$

!!! example "计数类通用模板"

    ```cpp
    long long countSubarrays(vector<int>& nums, int target) {
      long long total_count = 0;
      // cur 存储 {运算结果, 出现次数}
      // 这里的 cur 相当于最值问题中被原地修改的状态集
      vector<pair<int, int>> cur; 

      for (int x : nums) {
          for (auto& p : cur) {
              p.first = op(p.first, x);  // 增量更新：所有历史状态与新元素 x 运算
          }
          cur.push_back({x, 1});    // 加入当前单元素子数组, 也可以记录下标

          // 原地去重，利用单调性，将运算结果相同的项进行合并
          int k = 0;
          for (int i = 1; i < cur.size(); i++) {
              if (cur[i].first == cur[k].first) {
                  cur[k].second += cur[i].second; // 合并计数
              } else {
                  cur[++k] = cur[i]; // 移动到下一个去重位置
              }
          }
          cur.resize(k + 1); // 动态收缩列表长度至 O(log M)

          // 统计贡献
          for (const auto& [val, cnt] : cur) {
              if (val == target) {
                  total_count += cnt;
              }
          }
      }
      return total_count;
    }
    ```

在处理计数类问题时，根据查询类型的不同，状态压缩的实现可以分为两种主流模式：

1. 局部列表与原地合并 (List + 去重)： 维护一组按顺序排列的状态，在每次引入新元素后，将运算结果相同的相邻项进行原地合并。这是空间利用率最高、最通用的底层实现。
2. 全局映射表聚合 (Map 统计)： 如果题目涉及多重查询（例如：给定多个目标值 $k_1, k_2, \dots, k_m$，分别询问其出现的子数组个数），仅依靠局部列表无法高效回答。此时需要引入全局映射表（如 C++ 中的 std::unordered_map）：
      1. 依然使用 cur 列表维护当前右端点的压缩状态。
      2. 在每一轮右端点迭代结束时，将 cur 中所有状态的出现次数累加到全局映射表 global_count 中。
      3. 最终通过 $O(1)$ 的哈希查找即可响应任意数值的频次查询。此模式在 GCD 计数题中最为常见。

???+ note "[3209. 子数组按位与值为 K 的数目](https://leetcode.cn/problems/number-of-subarrays-with-and-value-of-k/){target=_blank}"

    给你一个整数数组 $nums$ 和一个整数 $k$，请你返回 $nums$ 中有多少个子数组满足：子数组中所有元素按位 AND 的结果为 $k$ 。

    ```cpp
    class Solution {
    public:
        long long countSubarrays(vector<int>& nums, int k) {
            long long ans = 0;
            for (int i = 0; i < nums.size(); i++) {
                int x = nums[i];
                for (int j = i - 1; j >= 0 && (nums[j] & x) != nums[j]; j--) {
                    nums[j] &= x;
                }
                ans += upper_bound(nums.begin(), nums.begin() + i + 1, k) -
                      lower_bound(nums.begin(), nums.begin() + i + 1, k);
            }
            return ans;
        }
    };
    ```

??? note "[CF 475D - CGCDSSQ](https://codeforces.com/problemset/problem/475/D){target=_blank}"

    给定长度为 $n$ 的整数序列 $a_1,\dots,a_n$，以及 $q$ 次询问 $x_1,\dots,x_q$。  
    对于每个询问 $x$，需要统计有多少个子数组 $[l, r]$ 满足：

    $$
    \gcd(a_l, a_{l+1}, \dots, a_r) = x
    $$

    也就是询问每个值作为某个子数组 $\gcd$ 的出现次数。

    ??? hint "GCD + Log Trick + 全局计数"

        这是 Log Trick 的典型计数题。

        固定右端点 $r$，考虑所有以 $r$ 结尾的子数组。随着左端点不断向左扩展，区间 $\gcd$ 只会单调递减，而且每次发生变化时都会变成原值的真因子，因此不同 $\gcd$ 的种类数只有 $O(\log V)$，其中 $V = \max(a_i)$。

        因此可以维护一个压缩状态：

        $$
        \text{cur} = [(g_1, c_1), (g_2, c_2), \dots]
        $$

        其中 $(g_i, c_i)$ 表示当前所有以 $r$ 结尾的子数组中，$\gcd = g_i$ 的有 $c_i$ 个。

        当加入新元素 $a_r$ 时：

        1. 先把上一个右端点的所有状态都与 $a_r$ 取一次 $\gcd$。
        2. 相邻相同的 $\gcd$ 合并计数。
        3. 再加入长度为 $1$ 的新区间 $[r, r]$。
        4. 将这一轮得到的所有 $(g, c)$ 累加到全局哈希表 `results[g]` 中。

        这样预处理完成后，每个询问只需输出 `results[x]` 即可。

        时间复杂度为 $O(n \log V)$ 到 $O(n \log^2 V)$（取决于状态维护方式），询问复杂度为 $O(1)$ 或 $O(\log V)$。

    === "map 状态转移版"

        用 `map` 直接维护当前右端点下每种 $\gcd$ 的出现次数，写法直接，但常数略大。

        ```cpp
        --8<-- "code/Trick/LogTrick/CF475D_1.cpp"
        ```

    === "vector 压缩版"

        利用相同 $\gcd$ 会成段出现这一性质，用有序状态数组原地合并，常数更优，也是更常见的 Log Trick 写法。

        ```cpp
        --8<-- "code/Trick/LogTrick/CF475D_2.cpp"
        ```

## 适用场景


当遇到符合以下特征的问题时，应首选 Log Trick 优化机制：

1. 问题域：连续子数组问题。
2. 运算特征：非加减乘运算，而是 OR、AND、GCD 或 LCM 等具备收敛性的非递减/非递增运算。
3. 数据规模：数组规模处于 $n \ge 10^5$ 量级，禁止使用 $O(n^2)$ 暴力枚举，且存在明确的值域上限 $M$。

核心判断逻辑在于：固定某一端点后，所有关联子数组的运算结果是否会在极短的步骤内收敛为极少量的离散值。
> 所有以它结尾的子数组结果，真的会有很多种吗？

注意：

1. 数据溢出限制：计数问题的累加结果通常属于 $O(n^2)$ 规模，变量声明必须使用 long long 或对应语言的 64 位整型。在应用到最小公倍数 (LCM) 时，数值极易呈指数级暴涨，需结合题目上限增加溢出保护机制。
2. 剪枝误用：切勿在“计数类问题”中应用最值问题的提前剪枝逻辑，否则会导致相同状态的重复计次丢失。
3. 内存覆盖：原地 In-place 算法具备破坏性。若涉及多轮查询或其他逻辑依赖原始数据结构，必须提前申请副本。

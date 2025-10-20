---
title: 摩尔投票
tags:
  - Moore Voting
  - Misra-Gries Algorithm
---

# 摩尔投票

摩尔投票算法是一种用于在数组中寻找出现次数超过一半的元素的高效算法。该算法的核心思想是通过抵消不同元素的出现次数，最终找到可能的候选元素。

???+ note "[多数元素](https://leetcode.cn/problems/majority-element/description/){target=_blank}"

    ```cpp
    class Solution {
     public:
      int majorityElement(vector<int> &nums) {
        int candidate = nums[0];
        int count     = 1;
        for (int num : nums) {
          if (count == 0) { candidate = num; }
          count += (num == candidate) ? 1 : -1;
        }
        return candidate;
      }
    };
    ```

## Misra-Gries 算法

Misra-Gries 算法是一种用于在数据流中寻找频繁元素的算法。与摩尔投票算法类似，它也使用了投票的思想，但在处理数据流时更加高效。

$$
\begin{aligned}
&\textbf{Algorithm } \text{Misra-Gries}\\
&\quad t,d \gets \varnothing, 0\\
&\quad \textbf{for } i=0 \text{ to } n-1 \textbf{ do}\\
&\qquad \textbf{if } b_i \notin t \textbf{ then } t \gets t \cup \{b_i\},\ d \gets d+1\\
&\qquad \textbf{else } t \gets t \cup \{b_i\},\ d \gets d\\
&\qquad \textbf{if } d = k \textbf{ then } \text{delete } k \text{ distinct values from } t;\ \text{update } d\\
&\quad \textbf{end for}
\end{aligned}
$$

等价实现：

$$
\begin{aligned}
&\textbf{Algorithm } \text{Misra-Gries}\\
&\quad T \leftarrow \varnothing,\ \text{counter map } C \text{ (empty)}\\
&\quad \textbf{for } i=0 \text{ to } n-1 \textbf{ do}\\
&\qquad \textbf{if } b_i \in T \textbf{ then } C[b_i] \leftarrow C[b_i]+1\\
&\qquad \textbf{else if } |T| < k-1 \textbf{ then } T \leftarrow T \cup \{b_i\},\ C[b_i]\leftarrow 1\\
&\qquad \textbf{else } \text{for each } x\in T:\ C[x]\leftarrow C[x]-1;\ \text{if } C[x]=0 \text{ then } T\leftarrow T\setminus\{x\}\\
&\quad \textbf{end for}
\end{aligned}
$$

!!! tip "寻找所有出现超过 $n/k$ 次的元素"

    维护 $k-1$ 个候选元素及其计数器，因为最多只能有 $k-1$ 个元素满足该条件。

???+ note "[多数元素](https://leetcode.cn/problems/majority-element/description/){target=_blank}"

    ```cpp
    class Solution {
    public:
      vector<int> majorityElement(vector<int> &nums) {
        int k = 3;  // Majority threshold is n/k
        vector<pair<int, int>> candidates(k - 1, {0, 0});
        for (int num : nums) { UpdateMajority(candidates, num); }
        vector<int> result;
        result.reserve(candidates.size());
        for (auto [candidate, count] : candidates) {  // verify candidates
          if (count == 0) { continue; }
          count = 0;
          for (int num : nums) {
            if (num == candidate) { count++; }
          }
          if (count > nums.size() / k) { result.push_back(candidate); }
        }
        return result;
      }

      void UpdateMajority(vector<pair<int, int>> &candidates, int num) {
        for (auto &[candidate, count] : candidates) {  // Already a candidate
          if (candidate == num && count > 0) {
            count++;
            return;
          }
        }
        for (auto &[candidate, count] : candidates) {  // Find an empty candidate slot
          if (count == 0) {
            candidate = num;
            count     = 1;
            return;
          }
        }
        // Decrease count for all candidates
        for (auto &[candidate, count] : candidates) { count = max(0, count - 1); }
      }
    };
    ```

??? note "[Buratsuta 3](https://codeforces.com/contest/2149/problem/G){target=_blank}"

    查询区间内所有出现次数超过三分之一（$\lfloor \frac{r - l + 1}{3} \rfloor$）的元素。

    ```cpp
    --8<-- "code/MooreVoting/CF2149G.cpp"
    ```
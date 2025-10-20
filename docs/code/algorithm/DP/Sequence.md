---
title: 序列 DP
tags:
  - Max Sequence Sum
  - Longest Increasing Subsequence
  - LIS
---

# 序列型动态规划

## 最大子数组和

最大子数组和目标是在一个整数数组中找到一个（非空）连续子数组，使得该子数组的和最大。

???+ note "[最大子数组和](https://leetcode.cn/problems/maximum-subarray/description/){target=_blank}"

    === "标准"

        ```cpp
        class Solution {
         public:
          int maxSubArray(vector<int> &nums) {
            int n = nums.size();
            vector<int> dp(n);  // dp[i]表示以nums[i]结尾的最大子数组和
            dp[0]   = nums[0];
            int ans = dp[0];
            for (int i = 1; i < n; ++i) {
              dp[i] = max(nums[i], dp[i - 1] + nums[i]);
              ans   = max(ans, dp[i]);
            }
            return ans;
          }
        };
        ```

    === "优化"

        ```cpp
        class Solution {
         public:
          int maxSubArray(vector<int> &nums) {
            int n = nums.size();
            int dp = nums[0];  // dp表示以nums[i]结尾的最大子数组和
            int ans = dp;
            for (int i = 1; i < n; ++i) {
              dp  = max(nums[i], dp + nums[i]);
              ans = max(ans, dp);
            }
            return ans;
          }
        };
        ```

??? note "输出所有最大子数组位置"

    ```cpp
    vector<pair<int, int>> maxSubArray(vector<int> &nums) {
      int n   = nums.size();
      int ans = nums[0];
      // 记录最大子数组的起始位置和结束位置
      vector<pair<int, int>> positions;
      positions.emplace_back(0, 0);  // 当前最大子数组的起始位置和结束位置
      for (int i = 1, current_max = nums[0], current_start = 0; i < n; ++i) {
        if (current_max + nums[i] < nums[i]) {  // 放弃之前的子数组, 从i开始重新计数
          current_max   = nums[i];
          current_start = i;
        } else {
          current_max += nums[i];
        }
        if (current_max > ans) {  // 更新最大子数组和
          ans = current_max;
          // 更新最大子数组的起始位置和结束位置
          positions.clear();
          positions.emplace_back(current_start, i);
        } else if (current_max == ans) {
          // 记录当前最大子数组的起始位置和结束位置
          positions.emplace_back(current_start, i);
        }
      }
      return positions;
    }
    ```

???+ note "[环形子数组的最大和](https://leetcode.cn/problems/maximum-sum-circular-subarray/description/){target=_blank}"

    题目要求在一个环形数组中找到一个（非空）连续子数组，使得该子数组的和最大。

    ```cpp
    class Solution {
    public:
      int maxSubarraySumCircular(vector<int> &nums) {
        int n     = nums.size();
        int total = 0;  // 数组总和
        for (int num : nums) { total += num; }
        int max_sum = nums[0];  // 最大子数组和
        int min_sum = nums[0];  // 最小子数组和
        for (int i = 1, current_max = nums[0], current_min = nums[0]; i < n; ++i) {
          current_max = max(nums[i], current_max + nums[i]);
          max_sum     = max(max_sum, current_max);
          current_min = min(nums[i], current_min + nums[i]);
          min_sum     = min(min_sum, current_min);
        }
        // 如果数组全为负数, 则返回最大子数组和, 否则返回环形最大子数组和
        return total == min_sum ? max_sum : max(max_sum, total - min_sum);
      }
    };
    ```

## 最长递增子序列

最长递增子序列（$\text{Longest Increasing Subsequence}$, $\text{LIS}$）是指在一个给定的序列中，找到一个最长的子序列，使得该子序列中的元素按严格递增的顺序排列。注意，子序列不要求在原序列中是连续的。

???+ note "[最长递增子序列](https://leetcode.cn/problems/longest-increasing-subsequence/description/){target=_blank}"

    === "标准"

        ```cpp
        class Solution {
         public:
          int lengthOfLIS(vector<int> &nums) {
            int n = nums.size();
            vector<int> dp(n, 1);  // dp[i]表示以nums[i]结尾的最长递增子序列长度
            int ans = dp[0];
            for (int i = 1; i < n; ++i) {
              for (int j = 0; j < i; ++j) {
                if (nums[i] > nums[j]) { dp[i] = max(dp[i], dp[j] + 1); }
              }
              ans = max(ans, dp[i]);
            }
            return ans;
          }
        };
        ```

    === "优化"

        ```cpp
        class Solution {
         public:
          int lengthOfLIS(vector<int> &nums) {
            int n = nums.size();
            // tails[i]表示长度为i+1的递增子序列的末尾元素的最小值
            vector<int> tails(n);  // tails数组是递增的
            int size = 0;          // tails数组的长度
            for (int num : nums) {
              auto it = lower_bound(tails.begin(), tails.begin() + size, num);
              if (it == tails.begin() + size) {
                tails[size++] = num;  // num比tails中所有元素都大, 则加入tails
              } else {
                *it = num;  // 更新tails[left], 保持tails数组递增
              }
            }
            return size;
          }
        };
        ```

??? note "最长非递减子序列"

    ```cpp
    int lengthOfLNDS(vector<int> &nums) {
      int n = nums.size();
      // tails[i]表示长度为i+1的非递减子序列的末尾元素的最小值
      vector<int> tails(n);  // tails数组是非递减的
      int size = 0;          // tails数组的长度
      for (int num : nums) {
        auto it = upper_bound(tails.begin(), tails.begin() + size, num);
        if (it == tails.begin() + size) {
          tails[size++] = num;  // num比tails中所有元素都大, 则加入tails
        } else {
          *it = num;  // 更新tails[left], 保持tails数组非递减
        }
      }
      return size;
    }
    ```

??? note "输出字典序最小的最长递增子序列"

    ```cpp
    vector<int> getLIS(vector<int> &nums) {
      int n = nums.size();
      vector<int> tails(n);             // tails[i]表示长度为i+1的递增子序列的末尾元素的最小值
      vector<int> indices(n, -1);       // indices[i]表示长度为i+1的递增子序列的末尾元素在nums中的索引
      vector<int> predecessors(n, -1);  // predecessors[i]表示nums[i]的前驱元素在nums中的索引
      int size = 0;                     // tails数组的长度
      for (int i = 0; i < n; ++i) {
        int num         = nums[i];
        auto it         = lower_bound(tails.begin(), tails.begin() + size, num);
        int index       = it - tails.begin();
        tails[index]    = num;                                  // 更新tails数组
        indices[index]  = i;                                    // 更新indices数组
        predecessors[i] = index > 0 ? indices[index - 1] : -1;  // 更新predecessors数组
        if (index == size) { size++; }
      }
      // 通过predecessors数组回溯得到最长递增子序列
      vector<int> lis(size);
      for (int i = indices[size - 1], j = size - 1; i >= 0; i = predecessors[i], --j) {
        lis[j] = nums[i];
      }
      return size;  // 返回最长递增子序列长度, lis数组即为最长递增子序列本身
    }
    ```

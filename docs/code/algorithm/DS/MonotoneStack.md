---
title: 单调栈
tags:
  - 单调栈
  - Monotone Stack
---

# 单调栈

单调栈（$\text{Monotone Stack}$）是一种特殊的数据结构，通常用于解决与数组或序列中元素的相对大小关系相关的问题。单调栈通过维护栈内元素的单调性（递增或递减）来高效地处理这些问题，常见应用包括寻找下一个更大或更小的元素、计算直方图中的最大矩形面积等。

单调栈主要分为两种类型：

1. 强序单调栈：栈内元素严格单调递增或递减
2. 弱序单调栈：栈内元素单调不增或不减

最常用的单调栈类型是强序单调栈。根据具体需求，可以选择适合的单调栈类型来解决问题。

???+ note "[下一个更大的元素](https://leetcode.cn/problems/daily-temperatures/description/){target=_blank}"

    给定一个整数数组 $temperatures$，表示每天的温度，返回一个数组 $answer$，其中 $answer[i]$ 是指在第 $i$ 天之后，才会有更高的温度。如果之后没有更高的温度，则在该位置用 $0$ 来代替。

    ```cpp
    vector<int> dailyTemperatures(vector<int> &temperatures) {
      int n = temperatures.size();
      vector<int> answer(n, 0);
      stack<int> st;  // 单调栈，存下标
      for (int i = 0; i < n; ++i) {
        // 弹出所有比当前元素小的元素, 并且更新它们的答案
        while (!st.empty() && temperatures[i] > temperatures[st.top()]) {
          answer[st.top()] = i - st.top();
          st.pop();
        }
        st.push(i);
      }
      // 清算栈，此处可省略，因为答案数组已初始化为 0
      // while (!st.empty()) {
      //   answer[st.top()] = 0;
      //   st.pop();
      // }
      return answer;
    }
    ```

## 左弱右强单调栈

在左弱右强单调栈中，栈内元素从栈底到栈顶是单调不增的（即左侧元素小于等于右侧元素）。这种结构适用于需要寻找每个元素左侧第一个小于等于它的元素和右侧第一个小于它的元素的问题。  

???+ note "寻找每个元素左侧第一个小于等于它的元素和右侧第一个小于它的元素"

    ```cpp
    void MonotoneStack(vector<int64_t> &nums) {
      int n = nums.size();
      vector<int> left(n, -1);  // 初始化, 后序无需清算栈
      vector<int> right(n, n);
      stack<int> st;  // 单调栈，存下标
      for (int i = 0; i < n; ++i) {
        // 右边强序：弹出所有比当前元素大的元素, 并且更新它们的右边界
        while (!st.empty() && nums[i] < nums[st.top()]) {
          right[st.top()] = i;
          st.pop();
        }
        // 左边弱序：栈顶可能是相等元素
        if (!st.empty()) { left[i] = st.top(); }
        st.push(i);
      }
    }
    ```

## 强序单调栈

在强序单调栈中，栈内元素从栈底到栈顶是严格单调递增的（即左侧元素小于右侧元素）。这种结构适用于需要寻找每个元素左侧第一个小于它的元素和右侧第一个小于它的元素的问题。  

???+ note "[寻找每个元素左右两侧第一个小于等于它的元素](https://www.nowcoder.com/practice/2a2c00e7a88a498693568cef63a4b7bb){target=_blank}"

    给定一个可能含有重复值的数组 $arr$，找到每一个 $i$ 位置左边和右边离 $i$ 位置最近且值比 $arr[i]$ 小的位置。返回所有位置相应的信息。

    === "单调栈修正"

        左弱右强单调栈可以正确处理右侧边界，但左侧边界无法处理重复元素，需要在主循环后对 $left$ 数组进行修正，跳过所有与当前元素相等的元素，直到找到第一个小于它的元素。

        ```cpp
        --8<-- "code/DS/MonotoneStack/smallest_left_right_1.cpp"
        ```
    === "双栈法"

        双栈法使用两个单调栈分别处理左侧和右侧的边界问题，避免了左弱右强单调栈中左侧边界修正的复杂性。

        ```cpp
        --8<-- "code/DS/MonotoneStack/smallest_left_right_2.cpp"
        ```

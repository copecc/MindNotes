---
title: 排序算法
tags:
  - 排序
  - sort
  - 基数排序
  - Radix Sort
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
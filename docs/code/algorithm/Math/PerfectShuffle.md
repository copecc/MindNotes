---
title: 完美洗牌
tags:
  - 洗牌
  - Perfect Shuffle
---

# 完美洗牌

完美洗牌（$\text{Perfect Shuffle}$）是指将一副牌均匀地分成两半，然后将两半牌交替合并在一起。  
完美洗牌分为两种：内洗牌 和 外洗牌。  

- 内洗牌（$\text{out-shuffle}$）：将下半部分的第一张牌放在最上面，然后交替放置上半部分和下半部分的牌  
- 外洗牌（$\text{in-shuffle}$）：将上半部分的第一张牌放在最上面，然后交替放置下半部分和上半部分的牌  

假设有 $2n$ 张牌，编号为 $0, 1, 2, \ldots, n-1, \vert n, n+1, n+2, \ldots, 2n-1$，则完美洗牌后的牌的编号为：  

内洗牌后的牌的编号为：
$\begin{aligned}
&n, 0, n+1, 1, n+2, 2, \ldots, 2n-1, n-1
\end{aligned}$

外洗牌后的牌的编号为：
$\begin{aligned}
&0, n, 1, n+1, 2, n+2, \ldots, n-1, 2n-1
\end{aligned}$

???+ note "完美洗牌（内洗）"

    === "辅助数组"

        ```cpp
        void in_shuffle(vector<int> &arr) {
          int n = arr.size();
          if (n % 2 != 0) { return; }
          vector<int> temp(arr);
          for (int i = 0; i < n / 2; ++i) {
            arr[2 * i]     = temp[n / 2 + i];
            arr[2 * i + 1] = temp[i];
          }
        }
        ```

    === "置换环"

        ```cpp
        void perfect_in_shuffle(vector<int> &arr) {
          int n = arr.size();
          if (n % 2 != 0) { return; }  // 仅处理偶数长度的数组
          // 长度为 3^k-1 的数组的子环起点为3^(k-1)
          // start 记录每个子环的起点, split 记录每个子环的长度
          // 任意偶数长度的数组都可以拆分为若干个 3^k-1 的子数组
          vector<int> start, split;
          for (int s = 1, p = 2; p <= n; s *= 3, p = s * 3 - 1) {
            start.push_back(s);
            split.push_back(p);
          }
          // 原地逆转
          auto reverse = [&](int left, int right) {
            while (left < right) { swap(arr[left++], arr[right--]); }
          };
          // 三次逆转实现区间旋转
          auto rotate = [&](int left, int mid, int right) {
            reverse(left, mid);
            reverse(mid + 1, right);
            reverse(left, right);
          };
          auto to
              = [&](int i, int l, int r) { return i <= (l + r) / 2 ? i + (i - l + 1) : i - (r - i + 1); };
          // 处理子环
          auto circle = [&](int l, int r, int i) {
            for (int j = 0; j <= i; ++j) {           // 枚举所有起点
              int init          = l + start[j] - 1;  // 当前子环起点
              int current       = init;
              int next          = to(current, l, r);
              int current_value = arr[current];
              while (next != init) {  // 处理循环链
                // 将current位置的值放到next位置, 然后记录原先next位置的值
                swap(arr[next], current_value);
                current = next;               // 更新current位置
                next    = to(current, l, r);  // 计算next位置
              }
              arr[init] = current_value;  // 将最后一个值放回起点
            }
          };

          for (int i = start.size() - 1, l = 0, r = n - 1; n > 0;) {
            if (split[i] <= n) {
              int mid = (l + r) / 2;  // 中点
              // 旋转区间 [l + split[i]/2, mid] 和 [mid + 1, mid + split[i]/2]
              // 使得区间 [l, l + split[i] - 1] 内的元素满足洗牌要求
              rotate(l + split[i] / 2, mid, mid + split[i] / 2);
              // 处理子环
              circle(l, l + split[i] - 1, i);
              l += split[i];  // 更新左边界
              n -= split[i];  // 更新剩余长度
            } else {
              i--;
            }
          }
        }
        ```
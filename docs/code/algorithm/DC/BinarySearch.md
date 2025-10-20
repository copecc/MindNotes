---
title: 二分查找
tags:
  - 二分查找
  - Binary Search
  - 三分
  - Ternary Search
---

# 二分查找

二分查找（$\text{Binary Search}$）是一种在有序数组中查找特定元素的高效算法。它通过反复将搜索区间减半来缩小查找范围，从而大大提高查找效率。  

## 二分查找

???+ note "升序数组中查找大于等于 $target$ 的第一个元素"

    === "闭区间"

        ```cpp
        int lower_bound(const vector<int> &nums, int target) {
          int left = 0, right = nums.size() - 1;
          while (left <= right) {  // 循环结束时有 L = R + 1
            int mid = left + (right - left) / 2;
            if (nums[mid] < target) {
              left = mid + 1;  // [mid+1, right]
            } else {
              right = mid - 1;  // [left, mid-1]
            }
          }
          return left;  // return right + 1;
        }
        ```

    === "左闭右开"

        ```cpp
        int lower_bound(const vector<int> &nums, int target) {
          int left = 0, right = nums.size();
          while (left < right) {  // 循环结束时有 L = R
            int mid = left + (right - left) / 2;
            if (nums[mid] < target) {
              left = mid + 1;  // [mid+1, right)
            } else {
              right = mid;  // [left, mid)
            }
          }
          return left;  // return right;
        }
        ```
    === "开区间"

        ```cpp
        int lower_bound(const vector<int> &nums, int target) {
          int left = -1, right = nums.size();
          while (left + 1 < right) {  // 循环结束时有 L + 1 = R
            int mid = left + (right - left) / 2;
            if (nums[mid] < target) {
              left = mid;  // (mid, right)
            } else {
              right = mid;  // (left, mid)
            }
          }
          return right;  // return left + 1;
        }
        ```

常见的四种情况转化为二分查找：  

- $\geq x$：使用上述二分查找即可
- $\gt x$：等价于 $\geq x + 1$， 对于非整数来说就是偏序中第一个大于 $x$ 的元素
- $\lt x$：等价于 $(\geq x) - 1$， 实际上就是第一个 $\geq x$ 的位置左边一个
- $\leq x$：等价于 $(\gt x) - 1$， 实际上就是第一个 $\gt x$ 的位置左边一个

???+ quote "[`std::lower_bound`](https://en.cppreference.com/w/cpp/algorithm/lower_bound.html){target=_blank}"
    > **Return value**  
    Iterator to the first element of the range `[first, last)` not ordered before value, or last if no such element is found.  
    <br>
    > Unlike `std::binary_search`, `std::lower_bound` does not require `operator<` or `comp` to be asymmetric (i.e., `a < b` and `b < a` always have different results). In fact, it does not even require `value < *iter` or `comp(value, *iter)` to be well-formed for any iterator iter in `[first, last)`.

    `std::lower_bound` 查找第一个**"大于等于"**目标值的元素位置。

???+ quote "[`std::upper_bound`](https://en.cppreference.com/w/cpp/algorithm/upper_bound.html){target=_blank}"
    > **Return value**  
    Iterator to the first element of the range `[first, last)` ordered after value, or last if no such element is found.  
    <br>
    > For any iterator iter in `[first, last)`, `std::upper_bound` requires `value < *iter` and `comp(value, *iter)` to be well-formed, while `std::lower_bound` requires `*iter < value` and `comp(*iter, value)` to be well-formed instead.

    `std::upper_bound` 查找第一个**"大于"**目标值的元素位置。

???+ quote "[`std::binary_search`](https://en.cppreference.com/w/cpp/algorithm/binary_search.html){target=_blank}"
    > **Return value**  
    `true` if there is an element in the range `[first, last)` that is equivalent to value, and `false` otherwise.  
    <br>
    > `std::binary_search` only checks whether an equivalent element exists. To obtain an iterator to that element (if exists), `std::lower_bound` should be used instead.

    `std::binary_search` 用于判断目标值是否存在于数组中。

利用 `std::lower_bound` 和 `std::upper_bound` 可以方便地实现二分查找。并且可以自定义`bool`函数`check(mid)` 来检查条件是否满足。  
对于**左边满足，右边不满足**这样的情况，只需要把 `check(mid)` 改成 `!check(mid)`。  

!!! tip
    查找满足最大化最小值和最小化最大值的题目，可以使用二分枚举答案。

## 三分查找

三分查找的思想类似于二分查找，但它将搜索区间分为三部分，从而在某些情况下可以更快地找到目标值。三分通常用于寻找函数的极值点（最大值或最小值）。

???+ note "[三分](https://www.luogu.com.cn/problem/P3382){target=_blank}"

    给定一个 $n$ 次多项式 $f(x) = a_n x^n + a_{n-1} x^{n-1} + ... + a_1 x + a_0$，以及一个区间 $[l, r]$，保证在范围 $[l,r]$ 内存在一点 $x$，使得 $[l,x]$ 上单调增，$[x,r]$ 上单调减。试求出 $x$ 的值。

    ```cpp
    --8<-- "code/DC/BinarySearch/P3382.cpp"
    ```

??? note "[Closest Moment](https://atcoder.jp/contests/abc426/tasks/abc426_e){target=_blank}"

    有两条线段，线段 $1$ 的起点为 $(s_{1x}, s_{1y})$，终点为 $(g_{1x}, g_{1y})$；线段 $2$ 的起点为 $(s_{2x}, s_{2y})$，终点为 $(g_{2x}, g_{2y})$。两条线段分别以匀速从起点运动到终点，求在运动过程中两条线段之间的最短距离。

    ??? hint

        设线段 $1$ 的长度为 $len_1$，线段 $2$ 的长度为 $len_2$，不妨设 $len_1 \geq len_2$。  
        线段 $1$ 按照线段 $2$ 的长度比例取中点 $mid$，则线段 $1$ 可以分为两部分：$(s_1, mid)$ 和 $(mid, g_1)$。  
        分别计算 $(s_1, mid)$ 与线段 $2$ 之间的最短距离，以及线段 $(mid, g_1)$ 与点 $g_2$ 之间的最短距离（此时较短线段已停止移动），二者的较小值即为所求。
        计算线段与线段之间的最短距离时，可以使用三分法。

        计算点与线段之间的最短距离时，可以变换为计算线段与原点之间的最短距离，即将线段的两个端点都减去该点的坐标。

    ```cpp
    --8<-- "code/DC/BinarySearch/abc426_e.cpp"
    ```
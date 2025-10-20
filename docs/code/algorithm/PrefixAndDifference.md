---
title: 前缀和与差分
tags:
  - prefix
  - difference
---

# 前缀和与差分

## 前缀和

前缀和（$\text{Prefix Sum}$）是指一个数组中从起始位置到当前位置的所有元素的累加和。前缀和数组可以快速计算任意子数组的和，避免重复计算。  

### 一维与二维前缀和

给定一个数组 $nums$，其前缀和数组 $sum$ 定义为：

$sum[i] = 
\begin{cases} 
0, & \text{if } i = 0 \\ 
sum[i-1] + nums[i-1], & \text{if } i > 0 
\end{cases}$  
<br>

使用前缀和数组计算子数组 $nums[i]$ 到 $nums[j]$ 的和可以通过以下公式实现：  
$sum(i, j) = 
\begin{cases} 
sum[j + 1], & \text{if } i = 0 \\ 
sum[j + 1] - sum[i], & \text{if } i > 0
\end{cases}$  
<br>

---

给定一个二维数组 $matrix$，其前缀和数组 $sum$表示从矩阵左上角 $(0, 0)$ 到位置 $(i-1, j-1)$ 的子矩阵的元素和，定义为：

$sum[i][j] = 
\begin{cases}
0, & \text{if } i = 0 \text{ or } j = 0 \\[2ex]
\begin{aligned}
& sum[i-1][j] + sum[i][j-1] - \\
& sum[i-1][j-1] + matrix[i-1][j-1],
\end{aligned} & \text{if } i > 0 \text{ and } j > 0
\end{cases}$  
<br>

使用二维前缀和数组计算子矩阵 $(row1, col1)$ 到 $(row2, col2)$ 的和可以通过以下公式实现：  
$\begin{aligned}
sum(row1, col1, row2, col2) &= sum[row2 + 1][col2 + 1] \\
&- sum[row1][col2 + 1] - sum[row2 + 1][col1] \\
&+ sum[row1][col1] \\
\end{aligned}$
<br>

???+ note "前缀和"
    === "一维前缀和"

        ```cpp
        struct NumArray {
          std::vector<int> sum;

          explicit NumArray(std::vector<int> &nums) {
            int n = nums.size();
            sum.resize(n + 1, 0);
            for (int i = 0; i < n; ++i) { sum[i + 1] = sum[i] + nums[i]; }
          }

          int sumRange(int left, int right) { return sum[right + 1] - sum[left]; }
        };
        ```

    === "二维前缀和"

        ```cpp
        struct NumMatrix {
          std::vector<std::vector<int>> sum;

          explicit NumMatrix(std::vector<std::vector<int>> &matrix) {
            int m = matrix.size();
            if (m == 0) { return; }
            int n = matrix[0].size();
            sum.resize(m + 1, std::vector<int>(n + 1, 0));
            for (int i = 0; i < m; ++i) {
              for (int j = 0; j < n; ++j) {
                sum[i + 1][j + 1] = sum[i][j + 1] + sum[i + 1][j] - sum[i][j] + matrix[i][j];
              }
            }
          }

          int sumRegion(int row1, int col1, int row2, int col2) {
            return sum[row2 + 1][col2 + 1] - sum[row1][col2 + 1] - sum[row2 + 1][col1] + sum[row1][col1];
          }
        };
        ```

### 高阶前缀和

高阶前缀和是指对前缀和数组进行多次前缀和计算得到的结果。  
定义如下：

- 一阶前缀和：$S_1(i) = a[1] + a[2] + \dots + a[i]$
- 二阶前缀和：$S_2(i) = S_1(1) + S_1(2) + \dots + S_1(i)$
- 高阶前缀和：$S_k(i) = \sum_{j=1}^i S_{k-1}(j)$

---
展开可得：$S_k(i) = \sum_{j=1}^i \left( \sum_{t=1}^j S_{k-2}(t) \right) = \dots = \sum_{j=1}^i a[j] \cdot P(k, i, j)$  
其中，$P(k, i, j)$ 表示组合系数（即 $a[j]$ 在 $S_k(i)$ 中的贡献次数）。  
具体地，$P(k, i, j) = \binom{i - j + k - 1}{k - 1}$，表示从 $i - j + k - 1$ 个位置中选择 $k - 1$ 个位置的组合数。  
通过展开 $P(k, i, j)$，可以将原式化为多项式形式，使用 $k$ 个[树状数组（BIT）](./DS/BIT.md) 维护以下前缀和：
$\sum a[j]$、 $\sum j \cdot a[j]$、$\sum j^2 \cdot a[j]$、...、$\sum j^{k-1} \cdot a[j]$，从而实现高阶前缀和的高效查询和更新。  

??? example "三阶前缀和的具体展开"

    展开 $P(3, i, j)$：

    $$
    P(3, i, j) = \frac{(i - j + 2)(i - j + 1)}{2} = \frac{1}{2}\left((i+1)(i+2) - (2i+3)j + j^2\right)
    $$
    
    因此，三阶前缀和 $S_3(i)$ 可表示为：

    $$
    \begin{aligned}
    S_3(i) & = \frac{1}{2} \sum_{j=1}^i a[j] \cdot \left((i+1)(i+2) - (2i+3)j + j^2\right) \\
    & = \frac{1}{2} \left( (i+1)(i+2) \sum_{j=1}^i a[j] - (2i+3) \sum_{j=1}^i j \cdot a[j] + \sum_{j=1}^i j^2 \cdot a[j] \right)
    \end{aligned}
    $$

## 差分数组

差分数组（$\text{Difference Array}$）是指一个数组中每个元素与其前一个元素的差值。差分数组可以快速进行区间更新操作，避免重复修改。  

### 一维与二维差分数组

给定一个数组 $nums$，其差分数组 $diff$ 定义为：  
$diff[i] = 
\begin{cases}
nums[0], & \text{if } i = 0 \\
nums[i] - nums[i-1], & \text{if } i > 0
\end{cases}$  
<br>

使用差分数组进行区间更新时，可以通过以下步骤实现：  
对于区间 $[i, j]$ 进行更新 $val$:  

1. 将 $diff[i]$ 增加 $val$，表示从位置 $i$ 开始的所有元素增加 $val$  
2. 将 $diff[j + 1]$ 减少 $val$，表示从位置 $j + 1$ 开始的所有元素减少 $val$  
3. 最后，通过前缀和计算出更新后的数组 $nums$  
<br>

---

对于二维数组 $matrix$，其差分数组 $diff$ 定义为：  
$diff[i][j] = 
\begin{cases}
matrix[0][0], & \text{if } i = 0 \text{ and } j = 0 \\[2ex]
matrix[i][0] - matrix[i-1][0], & \text{if } i > 0 \text{ and } j = 0 \\[2ex]
matrix[0][j] - matrix[0][j-1], & \text{if } i = 0 \text{ and } j > 0 \\[2ex]
\begin{aligned}
& matrix[i][j] - matrix[i-1][j] - \\
& matrix[i][j-1] + matrix[i-1][j-1],
\end{aligned} & \text{if } i > 0 \text{ and } j > 0
\end{cases}$  
<br>

使用二维差分数组进行区间更新时，可以通过以下步骤实现：  
对于子矩阵 $[(row1, col1), (row2, col2)]$ 进行更新:  

1. 将 $diff[row1][col1]$ 增加 $val$  
2. 将 $diff[row1][col2 + 1]$ 减少 $val$  
3. 将 $diff[row2 + 1][col1]$ 减少 $val$  
4. 将 $diff[row2 + 1][col2 + 1]$ 增加 $val$  
5. 最后，通过前缀和计算出更新后的数组 $matrix$  

???+ note "差分数组"
    === "一维差分数组"

        ```cpp
        struct NumArray {
          std::vector<int> diff;

          explicit NumArray(const std::vector<int> &nums) : diff(nums.size() + 1) {
            diff[0] = nums[0];
            for (int i = 1; i < nums.size(); ++i) { diff[i] = nums[i] - nums[i - 1]; }
            diff[nums.size()] = -nums.back();
          }

          void Modify(int left, int right, int val) {
            diff[left]      += val;
            diff[right + 1] -= val;
          }

          void Recover(std::vector<int> &nums) {
            nums[0] = diff[0];
            for (int i = 1; i < nums.size(); ++i) { nums[i] = nums[i - 1] + diff[i]; }
          }
        };
        ```

    === "二维差分数组"

        ```cpp
        struct NumMatrix {
          std::vector<std::vector<int>> diff;

          explicit NumMatrix(const std::vector<std::vector<int>> &matrix)
              : diff(matrix.size() + 1, std::vector<int>(matrix[0].size() + 1)) {
            for (int i = 0; i < matrix.size(); ++i) {
              for (int j = 0; j < matrix[i].size(); ++j) { Modify(i, j, i, j, matrix[i][j]); }
            }
          }

          void Modify(int row1, int col1, int row2, int col2, int val) {
            diff[row1][col1]         += val;
            diff[row1][col2 + 1]     -= val;
            diff[row2 + 1][col1]     -= val;
            diff[row2 + 1][col2 + 1] += val;
          }

          void Recover(std::vector<std::vector<int>> &matrix) {
            int m        = matrix.size();
            int n        = matrix[0].size();
            matrix[0][0] = diff[0][0];
            for (int i = 1; i < m; ++i) { matrix[i][0] = matrix[i - 1][0] + diff[i][0]; }
            for (int j = 1; j < n; ++j) { matrix[0][j] = matrix[0][j - 1] + diff[0][j]; }
            for (int i = 1; i < m; ++i) {
              for (int j = 1; j < n; ++j) {
                matrix[i][j] = diff[i][j] + matrix[i][j - 1] + matrix[i - 1][j] - matrix[i - 1][j - 1];
              }
            }
          }
        };
        ```

### 高阶差分数组

高阶差分数组是指对差分数组进行多次差分计算得到的结果。高阶差分数组可以快速进行复杂的区间更新操作。

给定一个数组 $nums$，其高阶差分数组 $D_k$ 定义为：  

- 一阶差分数组：$D_1[i] = nums[i] - nums[i-1]$  
- 二阶差分数组：$D_2[i] = D_1[i] - D_1[i-1]$  
- 高阶差分数组：$D_k[i] = D_{k-1}[i] - D_{k-1}[i-1]$

??? example "区间加等差数列的高阶差分数组更新"

    在区间内加上一个等差数列，可以通过对高阶差分数组进行相应的更新来实现。

    对数组 $nums$ 的区间 $[l, r]$ 加上首项为 $s$，公差为 $d$ 的等差数列，即相当于加上 $f(i) = s + (i - l) \cdot d$。

    假设初始数组为 $[0, 0, 0, 0, 0, 0, 0, 0]$，对区间 $[1, 5]$ 进行上述操作，即原数组变成 $[0, f(l), f(l+1), f(l+2), f(l+3), f(r=l+4), 0, 0]$。

    计算一阶差分数组：

    $$
    \begin{array}{c|l|l|c}
    \hline
    i & \text{计算公式} & \text{表达式} & \text{结果} \\
    \hline
    0 & 0 & 0 & 0 \\
    1 & f(l) - 0 & s & s \\
    2 & f(l+1) - f(l) & (s + d) - s & d \\
    3 & f(l+2) - f(l+1) & (s + 2d) - (s + d) & d \\
    4 & f(l+3) - f(l+2) & (s + 3d) - (s + 2d) & d \\
    5 & f(l+4) - f(l+3) & (s + 4d) - (s + 3d) & d \\
    6 & 0 - f(r = l+4) & -f(r) & -f(r) \\
    7 & 0 - 0 & 0 & 0 \\
    \hline
    \end{array}
    $$

    计算二阶差分数组：

    $$
    \begin{array}{c|l|l|c}
    \hline
    i & \text{计算公式} & \text{表达式} & \text{结果} \\
    \hline
    0 & 0 & 0 & 0 \\
    1 & f(l) - 0 & s - 0 & s \\
    2 & f(l+1) - f(l) - f(l) & (s + d) - s - s & d - s \\
    3 & f(l+2) + f(l) - 2f(l+1) & d - d & 0 \\
    4 & f(l+3) + f(l+1) - 2f(l+2) & d - d & 0 \\
    5 & f(l+4) + f(l+2) - 2f(l+3) & d - d & 0 \\
    6 & -f(r) - d & -(e + d) & -f(r+1) \\ 
    7 & 0 - (-f(r)) & 0 - (-f(r)) & f(r) \\
    \hline
    \end{array}
    $$

!!! tip "二阶差分数组的更新操作"

    1. $D_2[l] = D_2[l] + s$
    2. $D_2[l + 1] = D_2[l + 1] + d - s$
    3. $D_2[r + 1] = D_2[r + 1] - (d + e) = D_2[r + 1] - f(r + 1)$
    4. $D_2[r + 2] = D_2[r + 2] + e = D_2[r + 2] + f(r)$

??? note "[三步必杀](https://www.luogu.com.cn/problem/P4231){target=_blank}"

    给定一个长度为 $n$ 的数组，初始时所有元素均为 $0$。需要处理 $m$ 次操作，每次操作为：对区间 $[l, r]$ 上的每个元素加上首项为 $s$，末项为 $e$ 的等差数列。

    ```cpp
    #include <algorithm>
    #include <cstdint>
    #include <iostream>
    #include <vector>
    using namespace std;

    struct arithmetic {
      int n;
      vector<int64_t> d1, d2;

      explicit arithmetic(int n) : n(n), d1(n + 3, 0), d2(n + 3, 0) {}

      // [l, r] 加上首项 s、公差 d 的等差数列, 相当于 f(i) = s + (i - l) * d
      void add(int l, int r, int64_t s, int64_t d) {
        int64_t e  = s + (r - l) * d;
        d2[l]     += s;
        d2[l + 1] += d - s;
        d2[r + 1] -= d + e;  // 实际上就是末项的下一项
        d2[r + 2] += e;
      }

      // 恢复原始数组
      void recover(vector<int64_t> &nums) {
        for (int i = 1; i <= n; ++i) {
          d2[i]   = (d2[i] + d2[i - 1]);
          d1[i]   = (d1[i] + d1[i - 1] + d2[i]);
          nums[i] = (nums[i] + d1[i]);
        }
      }
    };

    int main() {
      ios::sync_with_stdio(false);
      cin.tie(nullptr);
      cout.tie(nullptr);
      int64_t n, m;
      cin >> n >> m;
      arithmetic a(n);
      for (int64_t i = 0; i < m; ++i) {
        int64_t l, r, s, e;
        cin >> l >> r >> s >> e;
        int64_t d = (e - s) / (r - l);
        a.add(l, r, s, d);
      }
      vector<int64_t> nums_a(n + 1, 0);
      a.recover(nums_a);
      int64_t xor_sum = 0, max_val = 0;
      for (int64_t i = 1; i <= n; ++i) {
        xor_sum ^= nums_a[i];
        max_val  = max(max_val, nums_a[i]);
      }
      cout << xor_sum << ' ' << max_val << '\n';
      return 0;
    }
    ```

??? example "区间加二次函数列的高阶差分数组更新"

    在区间内加上一个二次函数列，可以通过对高阶差分数组进行相应的更新来实现。

    对数组 $nums$ 的区间 $[l, r]$ 加上首项为 $s$，一次项系数为 $d$，二次项系数为 $c$ 的二次函数列，即相当于加上 $f(i) = s + (i - l) \cdot d + (i - l)^2 \cdot c$。

    假设初始数组为 $[0, 0, 0, 0, 0, 0, 0, 0]$，对区间 $[1, 4]$ 进行上述操作，即原数组变成 $[0, f(l), f(l+1), f(l+2), f(r = l+3), 0, 0, 0]$。

    计算一阶差分数组：

    $$
    \begin{array}{c|l|l|c}
    \hline
    i & \text{计算公式} & \text{表达式} & \text{结果} \\
    \hline
    0 & 0 & 0 & 0 \\
    1 & f(l) - 0 & s - 0 & s \\
    2 & f(l+1) - f(l) & (s + d + c) - s & d + c \\
    3 & f(l+2) - f(l+1) & (s + 2d + 4c) - (s + d + c) & d + 3c \\
    4 & f(l+3) - f(l+2) & (s + 3d + 9c) - (s + 2d + 4c) & d + 5c \\
    5 & 0 - f(r = l+3) & -f(r) = -(s + 3d + 9c) & -f(r) \\
    6 & 0 & 0 & 0 \\
    7 & 0 & 0 & 0 \\
    \hline
    \end{array}
    $$

    计算二阶差分数组：

    $$
    \begin{array}{c|l|l|c}
    \hline
    i & \text{计算公式} & \text{表达式} & \text{结果} \\
    \hline
    0 & 0 & 0 & 0 \\
    1 & D_1[1] - 0 & s - 0 & s \\
    2 & D_1[2] - D_1[1] & (d + c) - s & d + c - s \\
    3 & D_1[3] - D_1[2] & (d + 3c) - (d + c) & 2c \\
    4 & D_1[4] - D_1[3] & (d + 5c) - (d + 3c) & 2c \\
    5 & D_1[5] - D_1[4] & -f(r) - (d + 5c) & -f(r) - (d + 5c) \\
    6 & D_1[6] - D_1[5] & 0 - (-f(r)) & f(r) \\
    7 & 0 & 0 & 0 \\
    \hline
    \end{array}
    $$

    计算三阶差分数组：

    $$
    \begin{array}{c|l|l|c}
    \hline
    i & \text{计算公式} & \text{表达式} & \text{结果} \\
    \hline
    0 & 0 & 0 & 0 \\
    1 & D_2[1] - 0 & s - 0 & s \\
    2 & D_2[2] - D_2[1] & (d + c - s) - s & d + c - 2s \\
    3 & D_2[3] - D_2[2] & 2c - (d + c - s) & s + c - d \\
    4 & D_2[4] - D_2[3] & 2c - 2c & 0 \\
    5 & D_2[5] - D_2[4] & (-f(r) - (d + 5c)) - 2c = -(s + 4d + 16c) & -f(r+1) \\
    6 & D_2[6] - D_2[5] & f(r) + (f(r) + d + 5c + 2c) -2c & f(r) + f(r+1) -2c \\
    7 & 0 - D_2[6] & 0 - f(r) & -f(r) \\
    \hline
    \end{array}
    $$

!!! tip "三阶差分数组的更新操作"

    !!! warning "Not Tested"

    1. $D_3[l] = D_3[l] + s$
    2. $D_3[l + 1] = D_3[l + 1] + d + c - 2s$  
    3. $D_3[l + 2] = D_3[l + 2] + s + c - d$  
    4. $D_3[r + 1] = D_3[r + 1] - f(r + 1)$  
    5. $D_3[r + 2] = D_3[r + 2] + f(r + 1) + f(r) - 2c$  
    6. $D_3[r + 3] = D_3[r + 3] - f(r)$


??? note "[小w的糖果](https://ac.nowcoder.com/acm/contest/19483/H){target=_blank}"

    给定一个长度为 $n$ 的数组，初始时所有元素均为 $0$。需要处理 $m$ 次操作，每次操作有三种类型：

    1. 对区间 $[pos, n]$ 上的每个元素加上常数 $1$。
    2. 对区间 $[pos, n]$ 上的每个元素加上首项为 $1$，公差为 $1$ 的等差数列。
    3. 对区间 $[pos, n]$ 上的每个元素加上首项为 $1$ 的平方数列，即 $1, 4, 9, 16, \dots$。

    最终输出数组的所有元素，结果对 $10^9 + 7$ 取模。

    === "差分数组实现"

        ```cpp
        --8<-- "code/PrefixAndDifference/NC19483H_1.cpp"
        ```

    === "只关注首项"

        !!! hint

            由于操作均为在区间 $[pos, n]$ 上进行，因此只需关注首项的变化即可。因为大于 $n$ 的位置不会影响最终结果。

        ```cpp
        --8<-- "code/PrefixAndDifference/NC19483H_2.cpp"
        ```

!!! tip "动态区间更新与查询"

    前缀和与差分数组主要用于静态数组的区间查询与更新操作。对于动态数组，可以结合[线段树](./DS/SegmentTree.md)或[树状数组（BIT）](./DS/BIT.md)实现高效的区间查询与更新操作。
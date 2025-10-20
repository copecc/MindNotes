---
title: 排列与康托展开
tags:
  - 排列
  - permutation
  - 康托展开
  - Cantor Expansion
---

# 排列

排列（$\text{Permutation}$）是指从一组元素中，按照一定的顺序选取全部或部分元素所形成的有序序列。排列强调元素的顺序，因此不同的顺序被视为不同的排列。  

## 上一个排列与下一个排列

求解一组长度为 $n$ 的数的上一个排列:  

1. 找到最大下标 $i$，满足 $s[i] < s[i - 1] (1 \le i < n)$  
2. 找到最大下标 $j$，满足 $\forall k \in [i, j]: s[k] < s[i - 1] (1 \le j < n)$  
3. 交换下标为 $i-1$ 和 $j$ 处的两个字符  
4. 将下标 $i$ 开始的后缀反转  

???+ note "上一个排列"

    ```cpp
    #include <algorithm>
    #include <cstdint>
    #include <vector>
    using namespace std;

    vector<int64_t> prev_permutation(vector<int64_t> nums) {
      int n = nums.size();
      int i = n - 1;
      for (; i >= 1; --i) {
        if (nums[i] < nums[i - 1]) { break; }
      }
      // 已经是字典序最小的排列，没有更小的排列, 返回字典序最大的排列
      if (i == 0) {
        reverse(next(nums.begin(), i), nums.end());
        return nums;
      }
      int j = i;
      for (; j < n; ++j) { // 找到最后一个比 nums[i-1] 小的数
        if (nums[j] < nums[i - 1]) { continue; }
        break;
      }
      j -= 1; // j 回退到最后一个满足条件的位置
      swap(nums[i - 1], nums[j]);
      reverse(next(nums.begin(), i), nums.end());
      return nums;
    }
    ```

将上述算法中的比较式子不等号反转就能得到下一个排列:

???+ note "下一个排列"

    ```cpp
    #include <algorithm>
    #include <cstdint>
    #include <vector>
    using namespace std;

    vector<int64_t> next_permutation(vector<int64_t> nums) {
      int n = nums.size();
      int i = n - 1;
      for (; i >= 1; --i) {
        if (nums[i] > nums[i - 1]) { break; }
      }
      // 已经是字典序最大的排列，没有更大的排列, 返回字典序最小的排列
      if (i == 0) {
        reverse(next(nums.begin(), i), nums.end());
        return nums;
      }
      int j = i;
      for (; j < n; ++j) { // 找到最后一个比 nums[i-1] 大的数
        if (nums[j] > nums[i - 1]) { continue; }
        break;
      }
      j -= 1; // j 回退到最后一个满足条件的位置
      swap(nums[i - 1], nums[j]);
      reverse(next(nums.begin(), i), nums.end());
      return nums;
    }
    ```

## 康托展开

康托展开（$\text{Cantor Expansion}$）是一种将长度为 $n$ 的排列与 $n!$ 个整数之间建立双射关系的算法。  
前者到后者称为康托展开，后者到前者称为逆康托展开（$\text{Inverse Cantor Expansion}$）。  
---
康托展开将一个排列映射到一个整数，该整数表示该排列在字典序中的排名，从0开始计数。  

!!! example
    排列 $[3,1,2]$ 的康托展开为 $4$，因为在所有排列中，$[1,2,3],[1,3,2],[2,1,3],[2,3,1]$ 都排在它前面

对于排列 $P = [p_1, p_2, \dots, p_n]$，康托展开公式为：$C(P) = \sum_{i=1}^{n} (c_i * (n-i)!)$。  
其中 $c_i = \sum_{j=i+1}^{n} \mathbb{I}\left(p_j < p_i\right)$，表示在 $p_i$ 之后的元素中小于 $p_i$ 的元素个数。  
利用树状数组或线段树可以在 $O(n \log n)$ 时间内计算康托展开。  

???+ note annotate "[【模板】康托展开](https://www.luogu.com.cn/problem/P5367){target=_blank}"

    给定一个排列 $P = [p_1, p_2, \dots, p_n]$，求其康托展开 $C(P)$。

    ```cpp
    --8<-- "code/Math/Permutation/P5367.cpp"
    ```
    {.annotate }

    1.  如果从前向后处理，需要预处理阶乘数组<br>树状数组需要先标记所有元素，查询完成后再取消标记


## 逆康托展开

给定一个整数 $k$，求出字典序中排名为 $k$ 的排列。

对于排列 $P = [p_1, p_2, \dots, p_n]$，康托展开 $C(P) = \sum_{i=1}^{n} (c_i * (n-i)!)$，其中 $c_i$ 表示在 $p_i$ 之后的元素中小于 $p_i$ 的元素个数。  
逆康托展开的过程就是从 $k$ 中逐步提取 $c_i$ 的过程，然后根据 $c_i$ 构造排列。  
一般会先根据某个排列计算出**真实**排名，然后询问该排名之前或之后 $m$ 个排列。由于排名较大，因此需要使用**阶乘进制**来表示排名。  
{ .annotate }

???+ tip "阶乘进制"
    阶乘进制每一位的权值依次为 $0!, 1!, 2!, 3!, \dots, n!$。  
    第 $i(i=0,1,2,\dots,n)$ 位对应权值 $i!$，取值范围为 $[0, i]$。  
    进位处理与十进制类似。  

    !!! example
        十进制数 $463$ 转换为阶乘进制:  
        $463 = 3 * 5! + 4 * 4! + 1 * 3! + 0 * 2! + 1 * 1! + 0 * 0!$，因此阶乘进制表示为 $[3,4,1,0,1,0]$。

这个过程利用线段树（二分）可以在 $O(n \log n)$ 时间内完成。  

???+ note annotate "[火星人plus](https://www.luogu.com.cn/problem/U72177){target=_blank}"

    给定一个排列 $P = [p_1, p_2, \dots, p_n]$ 和一个整数 $k$，求字典序中排名为 $C(P) + k$ 的排列。

    ```cpp
    --8<-- "code/Math/Permutation/U72177.cpp"
    ```
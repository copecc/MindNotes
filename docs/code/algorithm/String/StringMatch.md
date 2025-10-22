---
title: 字符串匹配
tags:
  - 字符串匹配
  - String Match
  - KMP
  - Manacher
  - 扩展 KMP
  - AC 自动机
---

# 字符串匹配

字符串匹配在一个较大的文本字符串（$\text{text}$）中查找一个或多个较小的模式字符串（$\text{pattern}$）。常见的字符串匹配算法包括 $\text{KMP}$ 算法、$\text{Manacher}$ 算法、扩展 $\text{KMP}$ 算法和 $\text{AC}$ 自动机等。

## KMP

$\text{KMP}$（$\text{Knuth-Morris-Pratt}$）算法通过预处理模式字符串来加速匹配过程。

其核心思想是利用已经匹配的部分信息，避免重复比较。

定义一个 $next$ 数组，$next[i]$ 表示模式字符串中以位置 $i$ 结尾的子串的最长相等真前后缀(1)的长度。
{.annotate}

1. 真前缀：不包含最后一个字符（即 $i$ 位置）；真后缀：不包含第一个字符（即 $0$ 位置）  
   显然，位置 $0$ 处的字符没有真前后缀，$next[0] = 0$。

计算 $next$ 数组的步骤如下：

1. 初始化 $next[0] = 0$，表示第一个字符没有真前后缀
2. 使用两个指针 $i$ 和 $j$，其中 $i$ 遍历模式字符串，$j$ 记录当前匹配的前缀长度
3. 对于每个位置 $i$，如果 $pattern[i] == pattern[j]$，则 $next[i] = j + 1$(1)，并将 $j$ 增加 1  
    {.annotate}

    1. 更新前 $j$ 表示 $pattern[i-1]$ 的最长相等前后缀长度，说明 $pattern[0 \ldots j-1]$ 和 $pattern[i-j \ldots i-1]$ 相等。  
       当 $pattern[i]$ 等于 $pattern[j]$ 时，$pattern[0 \ldots j]$ 和 $pattern[i-j \ldots i]$ 也相等，因此最长相等前后缀长度为 $j+1$。

4. 如果不匹配且 $j > 0$，则将 $j$ 回退到 $next[j - 1]$(1)，继续尝试匹配
    {.annotate}

    1. 为了找到下一个可能的匹配位置，将 $j$ 回退到 $next[j - 1]$，即前一个可能的最长相等前后缀长度，继续尝试匹配。

5. 如果 $j == 0$ 且不匹配，则 $next[i] = 0$



???+ note "计算 $next$ 数组"

    ```cpp
    vector<int> get_next(const string &pattern) {
      vector<int> next(pattern.length());
      // j 表示当前匹配的前缀长度
      for (int i = 1, j = 0; i < pattern.length(); ++i) {
        while (j > 0 && pattern[i] != pattern[j]) { j = next[j - 1]; }
        if (pattern[i] == pattern[j]) { ++j; }
        next[i] = j;
      }
      return next;
    }
    ```

使用 $next$ 数组进行字符串匹配的步骤如下：

1. 计算 $next$ 数组
2. 遍历文本字符串 $text$，使用 $next$ 数组进行匹配
3. 如果 $text[i] == pattern[j]$，则同时增加 $i$ 和 $j$
4. 如果不匹配且 $j > 0$，则将 $j$ 回退到 $next[j - 1]$
5. 如果 $j == 0$ 且不匹配，则仅增加 $i$
6. 如果 $j$ 达到模式字符串的长度，表示找到一个匹配位置。如果允许重叠匹配，将 $j$ 回退到 $next[j - 1]$，继续寻找下一个匹配位置；如果不允许重叠匹配，将 $j$ 置为 $0$，继续寻找下一个匹配位置。

???+ note "$\text{KMP}$ 主函数"

    ```cpp
    int kmp_search(const string &text, const string &pattern) {
      vector<int> next = get_next(pattern);
      vector<int> positions;  // 记录匹配位置
      // i 表示 text 的匹配位置, j 表示 pattern 的匹配位置
      for (int i = 0, j = 0; i < text.length(); i++) {
        while (j > 0 && pattern[j] != text[i]) {  // 没有匹配则回退, 重新尝试匹配
          j = next[j - 1];  // 回退到下一个可能匹配的位置, 此时 [0,j-1] 已经匹配成功
        }
        // 匹配成功, 尝试匹配下一个字符
        if (text[i] == pattern[j]) { ++j; }
        // 匹配整个模式串, 回退到下一个可能匹配的位置
        if (j == pattern.length()) {
          positions.push_back(i - j + 1);
          j = next[j - 1];  // 如果不允许重叠 将j置为0
        }
      }
      return positions.size(); // 返回匹配次数
    }
    ```

??? note "[找出字符串中第一个匹配项的下标](https://leetcode.cn/problems/find-the-index-of-the-first-occurrence-in-a-string/description/){target=_blank}"

    ```cpp
    --8<-- "code/String/StringMatch/L28.cpp"
    ```

??? note "[最短回文串](https://leetcode.cn/problems/shortest-palindrome/description/){target=_blank}"

    若字符串 $s$ 的最长回文前缀的长度为 $len$，那么 $s[0 \ldots len-1]$ 是一个回文串，剩余部分（不属于回文前缀的部分）需要被反转并添加到 $s$ 的前面，才能使整个字符串成为回文串。

    ```cpp
    --8<-- "code/String/StringMatch/L214.cpp"
    ```

### 最小循环节

$\text{KMP}$ 算法还可以求出字符串的最小循环节(1)和最长重复子串(2)。
{.annotate}

1. 最小循环节：字符串可以表示为一个较短字符串的重复。

    !!! example
        字符串 "ababab" 的最小循环节是 "ab"，因为 "ababab" 可以表示为 "ab" 重复三次。

2. 最长重复子串：字符串中出现两次或多次的最长子串。

    !!! example
        在字符串 "banana" 中，最长重复子串是 "ana"。

字符串的最小循环节长度可以通过 $\text{KMP}$ 算法的 $next$ 数组来计算。设字符串长度为 $n$，则最小循环节的长度为 $n - next[n - 1]$。如果 $n$ 能被该长度整除，则该长度即为最小循环节的长度，否则最小循环节的长度为 $n$。

??? question "为什么？"
    
    $next[n-1] = k$ 表示字符串的前缀 $s[0 \ldots k-1]$ 与后缀 $s[n-k \ldots n-1]$ 相等，说明从位置 $n-k$ 开始，字符串重新出现相同的模式。也就是说每隔 $p = n-k$ 个字符，字符串的内容会出现相同的模式。  

    如果 $n$ 能被 $p$ 整除，就意味着字符串可以恰好分成若干个完整的周期：$s = \underbrace{s[0 \ldots p-1]}_{1} \underbrace{s[p \ldots 2p-1]}_{2} \cdots \underbrace{s[n-p \ldots n-1]}_{n/p}$

    若 $n$ 能被 $p$ 整除，说明字符串可以完整地由该长度为 $p$ 的循环节重复构成；否则最后一段不完整，最小循环节长度为 $n$。

???+ note "最小循环节"

    ```cpp
    string repetend_string(const string &s) {
      int n            = s.length();
      vector<int> next = get_next(s);
      int period       = n - next[n - 1];
      if (n % period == 0 && period != n) {
        return s.substr(0, period);  // s 的循环节
      }
      return "";  // s 没有循环节
    }
    ```

!!! example

    字符串 $s = \text{"ababab"}$，$n = 6, \; next[5] = 4$，因此 $p = n - next[5] = 2$。  

    因为 $6 \bmod 2 = 0$，字符串可以写成：$s = (ab)(ab)(ab)$。

    最小循环节长度为 $2$，循环节为 $ab$。

    ---

    字符串 $s = \text{"ababa"}$，$n = 5, \; next[4] = 3$，因此 $p = n - next[4] = 2$。

    因为 $5 \bmod 2 = 1 \ne 0$，说明字符串不能由长度为 $2$ 的循环节完整重复。
    
    最小循环节长度为整个字符串长度：$5$。


??? note "[Radio Transmission 无线传输](https://www.luogu.com.cn/problem/P4391){target=_blank}"

    一个字符串 $s1$ 由某个字符串 $s2$ 不断自我连接形成的（保证至少重复 $2$ 次）。字符串 $s2$ 的最短长度是多少？

    ```cpp
    --8<-- "code/String/StringMatch/P4391.cpp"
    ```

## Manacher

$\text{Manacher}$ 算法用于在字符串中查找最长回文子串，时间复杂度为 $O(n)$。其核心思想是利用回文的对称性来减少不必要的比较。

$\text{Manacher}$ 算法扩展字符串，将原字符串 $s$ 转换为一个新的字符串 $T$，在每个字符之间插入一个特殊字符(1)，以统一处理奇偶回文。
{.annotate}

1. 该特殊字符可以随意选择，不会影响回文性质和结果的计算，常用 $\#$ 或 $\$$。

$\text{Manacher}$ 算法利用一个辅助数组 $P$，其中 $P[i]$ 表示以位置 $i$ 为中心的最长回文子串的半径（不包括中心字符）。算法通过维护当前已知的最右回文边界 $right$ 和其对应的中心位置 $center$ 来加速计算。

---

扩展回文串和真实回文串有以下关系:

1. 每个回文子串的中心可能在一个字符上, 也可能在两个字符之间
2. 真实回文串的长度 等于 $半径-1$, 即 $p[i]-1$
3. 真实回文串的起始位置为 $(i - p[i] + 1) / 2$
4. 真实回文串的结束位置（不包括此位置字符）等于 $扩展回文串的右边界 / 2$

$\text{Manacher}$ 算法加速的原理:

当来到 $i$ 位置时, 根据和右边界 $right$ 的关系:

1. 如果 $i \geq right$, 直接暴力扩展
2. 如果 $i < right$, 则 $i$ 关于 $center$ 的对称点为 $j = 2*center - i$
    1. 如果 $p[j] + i < right$, 则 $p[i] = p[j]$
    2. 如果 $p[j] + i > right$, 则 $p[i] = right - i$
    3. 如果 $p[j] + i == right$, 则 $p[i] \geq right - i$, 需要暴力扩展

???+ note "[【模板】manacher](https://www.luogu.com.cn/problem/P3805){target=_blank}"

    ```cpp
    --8<-- "code/String/StringMatch/P3805.cpp"
    ```

## 扩展 KMP（Z 函数）

扩展 $\text{KMP}$（$\text{Extended KMP}$），也称为 $Z$ 函数，通过预处理字符串，快速计算出每个前缀在字符串中出现的位置。

对于一个字符串 $s$，定义 $Z[i]$ 为从位置 $i$ 开始的后缀与整个字符串的最长公共前缀的长度。

即 $Z[i] = \max \{ k : s[0 \ldots k-1] = s[i \ldots i+k-1] \}$。特别地，$Z[0] = \vert s \vert$。

$Z$ 函数的计算方法和 $\text{Manacher}$ 算法类似，利用已经计算出的信息来加速后续的计算。具体步骤如下：

1. 初始化 $Z[0] = n$，表示整个字符串与自身的最长公共前缀长度为 $n$
2. 使用两个指针 $l$ 和 $r$，表示当前已知的最长匹配区间的左右边界
3. 遍历字符串，对于每个位置 $i$：
   1. 如果 $i > r$，则从头开始匹配，计算 $Z[i]$，并更新 $l$ 和 $r$
   2. 如果 $i \leq r$，则利用对称性，计算 $Z[i]$ 的初始值为 $\min(Z[i - l], r - i + 1)$，然后尝试扩展匹配，并更新 $l$ 和 $r$（如果有扩展）

???+ note "Z 函数"

    ```cpp
    vector<int> get_z(const string &s) {
      int n = s.length();
      vector<int> z(n, 0);
      z[0] = n;
      // [l, r) 表示当前匹配区间，区间内 s[l...r-1] 与 s[0...(r-l-1)] 匹配
      for (int i = 1, l = 0, r = 0; i < n; ++i) {
        int len = r > i ? min(z[i - l], r - i) : 0;
        // 尝试扩展匹配区间
        while (i + len < n && s[len] == s[i + len]) { ++len; }
        z[i] = len;
        // 如果匹配区间扩展到 r 右边，更新 [l,r)
        if (i + len > r) {
          l = i;
          r = i + len;
        }
      }
      return z;
    }
    ```

$E$ 函数用于计算模式字符串 $pattern$ 在文本字符串 $text$ 中的匹配情况。定义 $E[i]$ 为从位置 $i$ 开始的后缀与模式字符串的最长公共前缀的长度。

即 $E[i] = \max \{ k : pattern[0 \ldots k-1] = text[i \ldots i+k-1] \}$。

$E$ 函数的计算具体步骤如下：

1. 初始化 $E[0] = 0$，表示模式字符串的第一个字符与文本字符串的第一个字符的匹配情况
2. 使用两个指针 $l$ 和 $r$，表示当前已知的最长匹配区间的左右边界
3. 遍历文本字符串，对于每个位置 $i$：
   1. 如果 $i > r$，则从头开始匹配，计算 $E[i]$，并更新 $l$ 和 $r$
   2. 如果 $i \leq r$，则利用对称性，计算 $E[i]$ 的初始值为 $\min(E[i - l], r - i + 1)$，然后尝试扩展匹配，并更新 $l$ 和 $r$（如果有扩展）

???+ note "E 函数"

    ```cpp
    vector<int> get_e(const string &text, const string &pattern) {
      int n = text.length(), m = pattern.length();
      vector<int> z = get_z(pattern);
      vector<int> e(n, 0);
      // 区间内 text[l..r-1] 与 pattern[0..(r-l-1)] 完全匹配
      for (int i = 0, l = 0, r = 0; i < n; ++i) {
        int len = r > i ? min(z[i - l], r - i) : 0;
        // 尝试扩展匹配区间
        while (i + len < n && len < m && text[i + len] == pattern[len]) { ++len; }
        e[i] = len;
        // 如果匹配区间扩展到 r 右边，更新 [l,r)
        if (i + len > r) {
          l = i;
          r = i + len;
        }
      }
      return e;
    }
    ```

??? note "[将单词恢复初始状态所需的最短时间 II](https://leetcode.cn/problems/minimum-time-to-revert-word-to-initial-state-ii/description/){target=_blank}"

    给定一个字符串 $word$ 和一个整数 $k$。每次操作中，你可以选择字符串的前 $k$ 个字符并将任意 $k$ 个字符加到字符串的末尾。返回将字符串恢复到其初始状态所需的最少操作次数。  

    ??? hint

        由于后面加的 $k$ 个字符是任意的，因此只要考虑每次移除前 $k$ 个字符之后是否能够完全和原字符串的前缀匹配即可。

    ```cpp
    --8<-- "code/String/StringMatch/L3031.cpp"
    ```

## AC 自动机
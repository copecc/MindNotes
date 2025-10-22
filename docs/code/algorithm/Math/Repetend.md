---
title: 循环节
tags:
  - 循环节
  - 循环小数
  - reptend
---

# 循环节

## 分数的循环节

循环节（$\text{Repetend}$）是指在循环小数中，重复出现的数字序列。循环节通常用括号括起来表示。  

!!! example 
    在小数 $0.333...$ 中，数字 $3$ 是循环节，因为它无限重复。  
    在小数 $0.142857142857...$ 中，数字 $142857$ 是循环节。  

???+ note "求循环节"

    求分数 $\frac{a}{n}$ 的循环节。  

    ```cpp
    #include <unordered_map>
    #include <vector>

    using namespace std;

    vector<int> repetend(int a, int n) {  // a为分子, n为分母
      vector<int> digits;                 // 存储小数部分的每一位
      // 使用哈希表记录每个余数第一次出现的位置
      // key: 余数, value: 该余数对应的小数部分的索引位置
      unordered_map<int, int> pos;
      int r   = a % n;  // 初始余数
      int idx = 0;
      while (r != 0 && !pos.contains(r)) {
        pos[r]  = idx++;
        r      *= 10;
        digits.push_back(r / n);  // 商的部分作为小数的一位
        r %= n;                   // 更新余数
      }
      // 没有循环节
      if (r == 0) { return {}; }
      return {digits.begin() + pos[r], digits.end()};
    }
    ```

??? note "[分数到小数](https://leetcode.cn/problems/fraction-to-recurring-decimal/description/){target=_blank}"

    给定两个整数，分别表示分数的分子 $numerator$ 和分母 $denominator$，以字符串形式返回小数。

    如果小数部分为循环小数，则将循环的部分括在括号内。

    ```cpp
    --8<-- "code/Math/Repetend/L166.cpp"
    ```



## 分数的循环节长度

对于一个既约分数 $\frac{a}{b}$ ，如果它的十进制表示是循环小数，那么它的循环节长度可以通过以下步骤确定：

1. 找到 $b$ 的质因数分解，记作 $b = 2^x \cdot 5^y \cdot p_1^{k_1} \cdot p_2^{k_2} \cdots p_m^{k_m}$，其中 $p_i$ 是不等于 $2$ 和 $5$ 的质数
2. 去掉 $b$ 中所有的 $2$ 和 $5$ 的因子，得到 $b' = p_1^{k_1} \cdot p_2^{k_2} \cdots p_m^{k_m}$
3. 如果 $b'$ 为 $1$，则 $\frac{a}{b}$ 是有限小数，没有循环节，循环节长度为 $0$
4. 否则，循环节长度 $k$ 是满足 $10^k \equiv 1 \mod b'$ 的最小正整数 $k$，也即 $10$ 在模 $b'$ 下的最小正整数阶

!!! abstract "证明"

    余数循环等价于 $(r * 10^k) \bmod b' = r$，因 $b'$ 与 $10$ 互质，等价于 $10^k \equiv 1 \mod b'$。

!!! tip
    数循环节的本质：模拟除法时余数会重复，导致小数部分循环。  
    分子 $a$ 只影响小数内容，不影响循环节长度，循环节长度只由分母 $b$ 决定。  

!!! question "如何求 $k$"

    由于 $b'$ 与 $10$ 互质，$10^k - 1$ 的因子中不包含 $2$ 和 $5$，因此 $10^k \equiv 1 \pmod{b'}$ 的最小正整数 $k$（即 $10$ 在模 $b'$ 下的最小正整数阶）必定是 $b'$ 的一个因子。

    根据数论，模 $b'$ 的乘法群的阶为 $\varphi(b')$，而 $10$ 的阶 $k$ 必定是 $\varphi(b')$ 的一个正因子。因此，只需枚举 $\varphi(b')$ 的所有正因子 $d$，找到最小满足 $10^d \equiv 1 \pmod{b'}$ 的 $d$，即可确定循环节长度 $k$。

??? note "[循环节](https://www.nowcoder.com/questionTerminal/b1b3a5610b3147fb80eb1c273983e936){target=_blank}"

    给定一个正整数 $n$，求 $\frac{1}{n}$ 的循环节长度。  

    ```cpp
    --8<-- "code/Math/Repetend/NCrepetend.cpp"
    ```

!!! tip "求任意进制（$b$ 进制）下分数 $\frac{a}{n}$ 的小数循环节长度"

    设 $b$ 和 $n$ 的最大公约数为 $g$，则 $n$ 可以表示为 $n = g * n'$，其中 $n'$ 与 $b$ 互质。  
    在 $b$ 进制下，分母 $n$ 中所有与 $b$ 共享的质因子（即 $b$ 的质因子）只影响有限小数部分，不影响循环节。  
    则 $\frac{a}{n}$ 的循环节长度等于 $\frac{a}{n'}$ 的循环节长度。  
    循环节长度 $k$ 是满足 $b^k \equiv 1 \mod n'$ 的最小正整数 $k$。  

## 字符串循环节 

见 [KMP 算法](../String/StringMatch.md#最小循环节)。
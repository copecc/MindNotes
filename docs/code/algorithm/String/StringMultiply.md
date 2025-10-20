---
title: 字符串乘法
tags:
  - multiply
---

# 字符串乘法

## 模拟笔算乘法

对于长为 $m$ 和 $n$ 的整数字符串 $num1$，$num2$，考虑第 $i$ 位和第 $j$ 位的乘积，产生乘积个位应该落在索引 $i+j+1$ 处，十位在索引 $i+j$ 处。  
再考虑进位，那么应该将索引 $i+j+1$ 处置为 $(num1[i]*num2[j]+carry) \% 10$，然后索引 $i+j$ 处记上进位 $(num1[i]*num2[j]+carry) / 10$。

???+ notes "模拟笔算"

    ```cpp
    std::string multiply(const std::string &num1, const std::string &num2) {
      int m = num1.size(), n = num2.size();
      if (num1 == "0" || num2 == "0") { return "0"; }
      std::vector<int> res(m + n, 0);
      for (int i = m - 1; i >= 0; i--) {
        for (int j = n - 1; j >= 0; j--) {
          // 倒序相乘，两个循环
          int mul = (num1[i] - '0') * (num2[j] - '0');
          int p1 = i + j, p2 = i + j + 1;
          int sum  = mul + res[p2];
          res[p2]  = sum % 10;
          res[p1] += sum / 10;
        }
      }
      // 或者直接用string流(stringstream)来拼接
      // 注意跳过前导0
      std::string result;
      for (int num : res) {
        if (!(result.empty() && num == 0)) { result.push_back(num + '0'); }
      }
      return result.empty() ? "0" : result;
    }
    ```

## 逆向考虑

对于结果中的第 $i$ 位（假设从右数起），由所有满足 $j+q=i$ 的位相乘后相加得到（再加上 $i-1$ 位的进位）。

???+ notes "逆向"

    ```cpp
    #include <algorithm>

    std::string multiply(const string &num1, const string &num2) {
      // 注意这里m,n和上面说明的字符串长度不一样，方便计算下标
      int m = num1.length() - 1, n = num2.length() - 1, carry = 0;
      std::string product;
      for (int i = 0; i <= m + n || carry; ++i) {
        // 只需考虑m+n位，carry是为了考虑最后一些进位
        for (int j = max(0, i - n); j <= min(i, m); ++j){
          // 从右起第j位对应的下标就是m-j;计算q=i-j,对应坐标为n-(i-j)
          carry += (num1[m - j] - '0') * (num2[n - i + j] - '0');
        }
        product += carry % 10 + '0';
        carry   /= 10;  // 保留进位
      }
      std::reverse(product.begin(), product.end());
      return product;
    }
    ```

考虑 $j$ 的取值范围: $j$ 可以从 $0$ 开始，但也要满足加上 $q$ 的最大值 $n$ 后能达到 $i$ ,也即至少为 $i-n$，因此 $i$ 应该取二者的最大值开始；$j$ 不能超过 $i$，也不能超过 $num1$ 本身长度。
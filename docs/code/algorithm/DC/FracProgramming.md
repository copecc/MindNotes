---
title: 分数规划
tags:
  - 分数规划
  - Fractional Programming
---

# 分数规划

分数规划（$\text{Fractional Programming}$）是一类优化问题，其目标函数是变量的分数形式。  
其形式化表述是，给出 $a_i$ 和 $b_i$，求一组 $w_i\in\{0,1\}$，最小化或最大化

$$
\frac{\sum_{i=1}^{n} a_i \times w_i}{\sum_{i=1}^{n} b_i \times w_i}
$$

$01$分数规划的核心：  

- 数据转化成结余表达式的形式，当确定一个比值 $x$，就能在最优决策下计算出结余和
- 最终希望获得在最优决策下，当结余和最接近 $0$ 时，$x$ 的值最优
- 最优决策判定：通常可以根据贪心选择、$01$背包、最小生成树等方法进行判定

常见应用：最大平均值（$\forall b_i = 1$）、最大性价比、最大比率等组合优化问题。  

## 二分法求解

分数规划问题可以通过二分法来求解。 设目标值为 $mid$，则问题转化为判断是否存在一组 $w_i$ 使得

$$
\frac{\sum_{i=1}^{n} a_i \times w_i}{\sum_{i=1}^{n} b_i \times w_i} \geq mid \\
\Longrightarrow
\sum_{i=1}^{n} a_i \times w_i \geq mid \times \sum_{i=1}^{n} b_i \times w_i
\Longrightarrow
\sum_{i=1}^{n} (a_i - mid \times b_i) \times w_i \geq 0
$$

通过二分法不断调整 $mid$ 的值，直到找到最优解。  

??? note "[Dropping Test](https://www.luogu.com.cn/problem/P10505){target=_blank}"

    给定 $n$ 个数据，每个数据有 $(a, b)$ 两个值，都为整数并且非负。舍弃掉 $k$ 个数据，希望让剩下数据 $\frac{\sum a}{\sum b}$ 尽量大。如果剩下数据所有 $b$ 的和为 $0$，认为无意义。结果乘以 $100$，小数部分四舍五入的整数结果返回。

    ```cpp
    --8<-- "code/DC/FP/P10505.cpp"
    ```

??? note "[Average and Median](https://atcoder.jp/contests/abc236/tasks/abc236_e){target=_blank}"

    给定 $n$ 个整数，相邻两个位置的数必须至少选择一个，求最大平均值和最大中位数。  

    ```cpp
    --8<-- "code/DC/FP/abc236_e.cpp"
    ```

!!! tip annotate
    - 最大平均值：转化为 $\sum (a_i - mid) \geq 0$，二分 $mid$。
    - 最大上中位数(1)：通过将元素值根据与中位数的大小关系转化为 $1$ 或 $-1$，判断哪一侧数量更多。

1. 下中位数：$\le mid$ 的数至少占一半  
   上中位数：$\ge mid$ 的数至少占一半  
---
title: 回溯
tags:
  - hot100
  - 回溯
---

# 回溯

回溯题的共同点是“枚举选择 + 及时撤销 + 剪枝”。Hot100 里的回溯题主要分成排列与子集、组合生成、网格搜索和约束搜索四类。

## 排列与子集

### 46. 全排列

!!! problem "[46. 全排列](https://leetcode.cn/problems/permutations/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个不含重复数字的整数数组 `nums`，请返回它的所有全排列。每个排列都应包含数组中的全部元素，且元素顺序不同就视为不同结果。

    输入是整数数组，输出是所有排列组成的列表。题目要求枚举出全部可能的顺序组合，因此结果数量会随元素个数阶乘增长。

排列问题的状态表示是“前面已经固定了哪些位置”。用交换把当前位置与后面的候选值互换，就能在原数组上直接生成每一种排列，回溯时再交换回来恢复现场。

???+ solution "46. 全排列"

    ```cpp
    --8<-- "code/leetcode/hot100/L46.cpp"
    ```

### 78. 子集

!!! problem "[78. 子集](https://leetcode.cn/problems/subsets/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个不含重复元素的整数数组 `nums`，请返回它的所有子集。子集不要求按固定顺序输出，只要覆盖所有可能的选取方案即可。

    输入是一个整数数组，输出是包含所有子集的列表。每个元素都只有“选”或“不选”两种状态，因此结果对应整棵二叉决策树的所有叶子与中间状态。

子集问题不要求顺序，所以每个位置只有“选”或“不选”两种分支。递归树的叶子对应完整答案，整棵搜索树本身就是所有子集的覆盖。

???+ solution "78. 子集"

    ```cpp
    --8<-- "code/leetcode/hot100/L78.cpp"
    ```

## 组合生成

### 17. 电话号码的字母组合

!!! problem "[17. 电话号码的字母组合](https://leetcode.cn/problems/letter-combinations-of-a-phone-number/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个只包含数字 `2` 到 `9` 的字符串，请返回它在电话按键映射下能够表示的所有字母组合。每个数字对应一组字符，组合结果需要覆盖所有按位选择的可能性。

    输入是数字字符串，输出是由若干字母组成的字符串列表。若输入为空，则应返回空结果。

这题就是按位展开的笛卡尔积。每个数字对应一个字符集合，回溯时只需要把当前位所有可选字符依次接到路径后面。

???+ solution "17. 电话号码的字母组合"

    ```cpp
    --8<-- "code/leetcode/hot100/L17.cpp"
    ```

### 39. 组合总和

!!! problem "[39. 组合总和](https://leetcode.cn/problems/combination-sum/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个无重复元素的候选数组 `candidates` 和目标值 `target`，请找出所有和为 `target` 的组合。候选数字可以重复使用，且同一个组合内部的数字顺序不影响结果。

    输入是候选数组和目标值，输出是所有满足条件的组合列表。题目要求枚举所有可行方案，同时避免重复组合被计入多次。

先排序再回溯，剪枝条件就很清楚：一旦当前候选值已经大于剩余目标，就可以停止向后搜索。允许重复使用同一个数，因此下一层递归仍然从当前下标开始。

???+ solution "39. 组合总和"

    ```cpp
    --8<-- "code/leetcode/hot100/L39.cpp"
    ```

### 22. 括号生成

!!! problem "[22. 括号生成](https://leetcode.cn/problems/generate-parentheses/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定整数 `n`，请生成所有由 `n` 对括号组成的合法括号串。合法括号串要求任意前缀中左括号数量都不少于右括号数量，且最终左右括号数量相等。

    输入是 `n`，输出是所有满足条件的字符串列表。题目关注的是完整枚举而不是单个构造结果，因此搜索过程需要同时维护左右括号的使用数量。

合法性可以转成一个前缀不变量：任意时刻已经使用的右括号数不能超过左括号数。只要同时维护 `open` 和 `close` 的数量，搜索过程中就不会产生非法前缀。

???+ solution "22. 括号生成"

    ```cpp
    --8<-- "code/leetcode/hot100/L22.cpp"
    ```

## 网格搜索

### 79. 单词搜索

!!! problem "[79. 单词搜索](https://leetcode.cn/problems/word-search/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个二维字符网格和一个单词 `word`，请判断该单词是否可以由网格中相邻格子中的字母按顺序拼出。相邻只允许上下左右四个方向，且同一个格子不能在同一次路径中重复使用。

    输入是字符网格和目标单词，输出是布尔值。题目要求沿网格做路径搜索，因此需要同时满足字符匹配和访问约束。

这题是标准的带回溯标记的 DFS。每一步只能走上下左右四个方向，并且同一个格子不能重复使用，所以搜索前要暂时标记，搜索结束后再恢复原值。

???+ solution "79. 单词搜索"

    ```cpp
    --8<-- "code/leetcode/hot100/L79.cpp"
    ```

## 约束搜索

### 131. 分割回文串

!!! problem "[131. 分割回文串](https://leetcode.cn/problems/palindrome-partitioning/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个字符串 `s`，请将其分割成若干个子串，使得每个子串都是回文串，并返回所有可能的分割方案。每种方案都要完整覆盖整个字符串，不能遗漏也不能重叠。

    输入是字符串，输出是所有满足条件的分割列表。题目本质上是在所有切分位置上做回溯枚举，并在每一步检查当前片段是否为回文。

搜索时维护的是切分位置，而不是最终答案本身。每次尝试一个新的结束位置前，先判断子串是否回文，能够把明显无效的分支提前剪掉。

???+ solution "131. 分割回文串"

    ```cpp
    --8<-- "code/leetcode/hot100/L131.cpp"
    ```

### 51. N 皇后

!!! problem "[51. N 皇后](https://leetcode.cn/problems/n-queens/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个整数 `n`，请在 `n x n` 的棋盘上放置 `n` 个皇后，使任意两个皇后都不能互相攻击。皇后可以攻击同一行、同一列以及两条对角线上的所有位置。

    输入是棋盘大小，输出是所有合法的棋盘摆法。题目要求枚举全部可行解，因此需要在搜索过程中持续维护列和对角线的冲突状态。

这题的剪枝条件是列和两条对角线不能重复占用。行按顺序递归时，每一行只需要尝试当前还没被冲突约束占用的列，搜索空间就会被大幅压缩。

???+ solution "51. N 皇后"

    ```cpp
    --8<-- "code/leetcode/hot100/L51.cpp"
    ```

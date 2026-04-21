---
title: 二叉树
tags:
  - hot100
  - 二叉树
---

# 二叉树

二叉树题目通常围绕三类不变量展开：遍历顺序、子树性质和层级关系。Hot100 里的题目基本可以归到 DFS、BFS 和 BST 性质验证三条主线。

## 遍历与层序

### 94. 二叉树的中序遍历

!!! problem "[94. 二叉树的中序遍历](https://leetcode.cn/problems/binary-tree-inorder-traversal/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定二叉树的根节点 `root`，请返回它的中序遍历结果。中序遍历的访问顺序为左子树、当前节点、右子树，返回值应按这个顺序收集所有节点值。

    输入只提供根节点，输出是一个按遍历顺序排列的整数数组；题目不要求修改树结构。

中序遍历的本质是“左子树、当前节点、右子树”的访问顺序。递归写法最直接，迭代写法则用栈模拟递归展开过程，当前节点沿左链一路下沉，回溯时再访问并转向右子树。

???+ solution "94. 二叉树的中序遍历"

    ```cpp
    --8<-- "code/leetcode/hot100/L94.cpp"
    ```

### 104. 二叉树的最大深度

!!! problem "[104. 二叉树的最大深度](https://leetcode.cn/problems/maximum-depth-of-binary-tree/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定二叉树的根节点 `root`，请返回树的最大深度。深度定义为从根节点到最远叶子节点经过的节点数量，因此空树的深度为 `0`。

    输入是二叉树根节点，输出是一个整数。题目只关心整棵树中最长的根到叶路径长度，不需要记录具体路径。

最大深度是典型的树形递归：根节点的深度等于左右子树最大深度的较大值再加一。这里不需要额外状态，问题本身就已经给出了递推关系。

???+ solution "104. 二叉树的最大深度"

    ```cpp
    --8<-- "code/leetcode/hot100/L104.cpp"
    ```

### 102. 二叉树的层序遍历

!!! problem "[102. 二叉树的层序遍历](https://leetcode.cn/problems/binary-tree-level-order-traversal/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定二叉树的根节点 `root`，请按从上到下、从左到右的顺序返回每一层的节点值。结果需要按层组织，外层表示层次，内层表示该层从左到右的节点值。

    输入只提供根节点，输出是二维数组；若树为空，则返回空结果。

层序遍历的核心是把“当前层”和“下一层”分开管理。队列里始终保存未处理节点，每轮固定取出当前层的节点数，处理完这一批后自然就进入下一层。

???+ solution "102. 二叉树的层序遍历"

    ```cpp
    --8<-- "code/leetcode/hot100/L102.cpp"
    ```

## 结构变换

### 226. 翻转二叉树

!!! problem "[226. 翻转二叉树](https://leetcode.cn/problems/invert-binary-tree/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定二叉树的根节点 `root`，请将整棵树翻转，也就是把每个节点的左子树和右子树互换，并返回翻转后的根节点。

    输入是二叉树根节点，输出是翻转后的树根。题目关注的是左右子树的对调关系，节点值本身不变。

这题只需要在每个节点处做一次左右交换，然后递归处理子树。因为左右子树互不依赖，所以可以直接用 DFS 递归完成。

???+ solution "226. 翻转二叉树"

    ```cpp
    --8<-- "code/leetcode/hot100/L226.cpp"
    ```

### 101. 对称二叉树

!!! problem "[101. 对称二叉树](https://leetcode.cn/problems/symmetric-tree/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定二叉树的根节点 `root`，判断这棵树是否关于中心轴对称。所谓对称，是指左子树与右子树在结构和节点值上都互为镜像。

    输入是树的根节点，输出是布尔值；只要有一处结构或值不满足镜像关系，就应当判定为不对称。

对称性要求左子树的左边界与右子树的右边界、左子树的右边界与右子树的左边界同时相等。因此递归比较时，参数必须成镜像配对，而不是简单比较同位置节点。

???+ solution "101. 对称二叉树"

    ```cpp
    --8<-- "code/leetcode/hot100/L101.cpp"
    ```

### 543. 二叉树的直径

!!! problem "[543. 二叉树的直径](https://leetcode.cn/problems/diameter-of-binary-tree/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定二叉树的根节点 `root`，请返回这棵树的直径。直径定义为树中任意两个节点之间最长路径所经过的边数，路径不一定经过根节点。

    输入只提供根节点，输出是一个整数。题目关注的是路径长度，计算时应以边数为准，而不是节点数。

直径不一定经过根节点，所以要在 DFS 过程中顺手维护全局答案。对每个节点来说，穿过它的最长路径长度就是左子树高度加右子树高度，递归返回的则是当前子树的高度。

???+ solution "543. 二叉树的直径"

    ```cpp
    --8<-- "code/leetcode/hot100/L543.cpp"
    ```

## BST 性质

### 108. 将有序数组转换为二叉搜索树

!!! problem "[108. 将有序数组转换为二叉搜索树](https://leetcode.cn/problems/convert-sorted-array-to-binary-search-tree/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一个按升序排列的整数数组 `nums`，请将其转换为一棵高度平衡的二叉搜索树并返回根节点。高度平衡要求每个节点左右子树的高度差尽可能小。

    输入是有序数组，输出是构造出的 BST 根节点。题目不要求唯一答案，只要生成的树满足 BST 性质并保持平衡即可。

平衡树的构造方式很直接：每次选数组中点作为根，左半部分递归生成左子树，右半部分递归生成右子树。中点切分保证左右规模尽量接近，因此树高保持在对数级。

???+ solution "108. 将有序数组转换为二叉搜索树"

    ```cpp
    --8<-- "code/leetcode/hot100/L108.cpp"
    ```

### 98. 验证二叉搜索树

!!! problem "[98. 验证二叉搜索树](https://leetcode.cn/problems/validate-binary-search-tree/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定二叉树的根节点 `root`，请判断它是否是一棵合法的二叉搜索树（BST）。BST 要求任意节点的左子树所有节点值都严格小于该节点，右子树所有节点值都严格大于该节点。

    输入是二叉树根节点，输出是布尔值。题目要求的是整棵树范围内的严格约束，而不是只比较父节点和直接孩子。

局部比较节点和直接孩子不够，因为 BST 的约束是整棵子树都必须落在一个严格区间内。递归时携带上下界，就能把“所有左子树节点都小于根、所有右子树节点都大于根”转成可检查的不变量。

???+ solution "98. 验证二叉搜索树"

    ```cpp
    --8<-- "code/leetcode/hot100/L98.cpp"
    ```

### 230. 二叉搜索树中第 K 小的元素

!!! problem "[230. 二叉搜索树中第 K 小的元素](https://leetcode.cn/problems/kth-smallest-element-in-a-bst/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定一棵二叉搜索树的根节点 `root` 和整数 `k`，请返回树中第 `k` 小的节点值。BST 的中序遍历结果天然按从小到大排序，因此答案对应有序序列中的第 `k` 个元素。

    输入包含树根和 `k`，输出是一个整数。题目默认 `k` 有效，能够在树的节点数范围内找到对应元素。

BST 的中序遍历天然是升序序列，所以只要按中序顺序访问节点并计数，数到第 `k` 个就能直接返回。迭代栈写法可以避免递归深度过大。

???+ solution "230. 二叉搜索树中第 K 小的元素"

    ```cpp
    --8<-- "code/leetcode/hot100/L230.cpp"
    ```

## 最近公共祖先

### 236. 二叉树的最近公共祖先

!!! problem "[236. 二叉树的最近公共祖先](https://leetcode.cn/problems/lowest-common-ancestor-of-a-binary-tree/description/?envType=study-plan-v2&envId=top-100-liked){target=_blank}"

    给定普通二叉树的根节点 `root`，以及树中两个节点 `p` 和 `q`，请返回它们的最近公共祖先。最近公共祖先是指同时是 `p` 和 `q` 祖先的节点中，层级最深的那个。

    输入提供树根和两个目标节点引用，输出应返回对应的节点对象本身，而不是节点值。题目不要求树是 BST，只依赖普通二叉树结构。

递归返回值表示“当前子树是否包含目标节点，若包含则返回对应的祖先信息”。当左右子树分别命中一个目标节点时，当前根就是公共祖先；若只有一侧命中，就把那一侧的结果向上返回。

???+ solution "236. 二叉树的最近公共祖先"

    ```cpp
    --8<-- "code/leetcode/hot100/L236.cpp"
    ```

---
title: 树的遍历
tags:
  - 遍历
  - Traverse
  - 重建二叉树
  - Build Tree
  - Morris 遍历
---

# 树的遍历

树的遍历是指按照某种顺序访问树中的所有节点。常见的树的遍历方式有深度优先遍历（$\text{DFS}$）和广度优先遍历（$\text{BFS}$）。

## 二叉树的非递归DFS

非递归 $\text{DFS}$ 使用栈来模拟递归过程。

???+ note "二叉树的非递归 $\text{DFS}$"

    ??? info "二叉树的节点定义"

        ```cpp
        struct TreeNode {
          int val;
          TreeNode *left;
          TreeNode *right;

          TreeNode() : val(0), left(nullptr), right(nullptr) {}

          explicit TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}

          TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
        };
        ```

    === "前序遍历"

        ```cpp
        vector<int> pre_order_traversal(TreeNode *root) {
          vector<int> traverse;
          stack<TreeNode *> st;
          while ((root != nullptr) || !st.empty()) {  // 如果根节点为空则从栈中取节点
            while (root != nullptr) {
              traverse.emplace_back(root->val);  // 先访问当前节点
              st.emplace(root);                  // 再将当前节点入栈, 方便回溯
              root = root->left;                 // 处理左子树
            }
            // 到达最左节点的左子树(nullptr)，回溯到上一个节点(也就是最左节点), 转向其右子树
            root = st.top()->right;
            st.pop();  // 弹出已经回溯的节点
          }
          return traverse;
        }
        ```

    === "中序遍历"

        ```cpp
        vector<int> in_order_traversal(TreeNode *root) {
          vector<int> traverse;
          stack<TreeNode *> st;
          while ((root != nullptr) || !st.empty()) {
            while (root != nullptr) {
              st.push(root);      // 先将当前节点入栈, 方便回溯
              root = root->left;  // 处理左子树
            }
            root = st.top();  // 到达最左节点的左子树(nullptr)，回溯到上一个节点(也就是最左节点)
            traverse.push_back(root->val);  // 访问该节点
            st.pop();                       // 弹出已访问节点
            root = root->right;             // 访问节点后，转向右子树
          }
          return traverse;
        }
        ```
    === "后序遍历"

        ```cpp
        vector<int> post_order_traversal(TreeNode *root) {
          vector<int> traverse;
          std::stack<TreeNode *> st;
          TreeNode *last = nullptr;  // 记录上一个访问的节点
          while (root != nullptr || !st.empty()) {
            while (root != nullptr) {
              st.emplace(root);   // 先将当前节点入栈, 方便回溯
              root = root->left;  // 处理左子树
            }
            // 到达最左节点的左子树(nullptr)，回溯到上一个节点(也就是最左节点)
            if (st.top()->right == nullptr || st.top()->right == last) {  // 右子树为空或已访问过
              last = st.top();
              traverse.emplace_back(st.top()->val);  // 访问该节点
              st.pop();                              // 弹出已访问节点
            } else {                                 // 右子树不为空且未访问过，转向右子树
              root = st.top()->right;
            }
          }
          return traverse;
        }
        ```

## 遍历序列重建二叉树

根据树的前序和中序遍历序列或后序和中序遍历序列来重建二叉树。注意，树中没有重复元素。

!!! warning "前序和后序遍历无法唯一确定一棵树"

    以下两棵树的前序遍历均为 $[1, 2, 3]$，后序遍历均为 $[3, 2, 1]$，但结构不同。

    ```mermaid
    graph TD
        %% === 第一棵树 ===
        A1(("1"))
        A2(("2"))
        A_null1(("null"))
        A_null2(("null"))
        A3(("3"))

        A1 --> A2
        A1 --> A_null1
        A2 --> A_null2
        A2 --> A3

        %% === 第二棵树 ===
        B1(("1"))
        B_null1(("null"))
        B2(("2"))
        B3(("3"))
        B_null2(("null"))

        B1 --> B_null1
        B1 --> B2
        B2 --> B3
        B2 --> B_null2

        %% === 样式设置 ===
        classDef nullNode fill:#eee,stroke:#ccc,color:#666;
        class A_null1,A_null2,B_null1,B_null2 nullNode;
    ```

    只有一个子树的节点无法确定子树挂在左边还是右边。

    !!! tip "唯一确定二叉树的条件"

        只有当二叉树中每个节点的度均为 $2$ 或 $0$（即真二叉树）时，前序和后序遍历序列才能唯一确定二叉树。若存在度为 $1$ 的节点，则无法唯一还原。

???+ note "根据遍历序列重建二叉树"

    ??? info "二叉树的节点定义"

        ```cpp
        struct TreeNode {
          int val;
          TreeNode *left;
          TreeNode *right;

          TreeNode() : val(0), left(nullptr), right(nullptr) {}

          explicit TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}

          TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
        };
        ```

    === "前序和中序遍历重建二叉树"

        [从前序与中序遍历序列构造二叉树](https://leetcode.cn/problems/construct-binary-tree-from-preorder-and-inorder-traversal/description/){target="_blank"}

        ```cpp
        TreeNode *build_tree(const vector<int> &pre_order, const vector<int> &in_order) {
          int root_index = 0;
          // 左闭右开区间简化std::find
          std::function<TreeNode *(int, int)> build = [&](int in_begin, int in_end) -> TreeNode * {
            if (in_begin >= in_end) { return nullptr; }
            auto *root = new TreeNode(pre_order[root_index]);
            root_index++;
            // 从中序遍历中找到和当前根节点对应的下标
            // 其左边就是左子树的中序遍历结果，右边就是右子树的中序遍历结果，注意第二个参数+1
            int in_index = find(in_order.begin() + in_begin, in_order.begin() + in_end, root->val)
                        - in_order.begin();
            // 先建左子树再建右子树
            root->left  = build(in_begin, in_index);
            root->right = build(in_index + 1, in_end);
            return root;
          };
          return build(0, in_order.size());
        }
        ```

    === "后序和中序遍历重建二叉树"

        [从中序与后序遍历序列构造二叉树](https://leetcode.cn/problems/construct-binary-tree-from-inorder-and-postorder-traversal/description/){target="_blank"}

        ```cpp
        TreeNode *build_tree(const vector<int> &post_order, const vector<int> &in_order) {
          int root_index = post_order.size() - 1;
          // 左闭右开区间简化std::find
          std::function<TreeNode *(int, int)> build = [&](int in_begin, int in_end) -> TreeNode * {
            if (in_begin >= in_end) { return nullptr; }
            auto *root = new TreeNode(post_order[root_index]);
            root_index--;
            // 从中序遍历中找到和当前根节点对应的下标
            // 其左边就是左子树的中序遍历结果，右边就是右子树的中序遍历结果，注意第一个参数+1
            int in_index = find(in_order.begin() + in_begin, in_order.begin() + in_end, root->val)
                        - in_order.begin();
            // 先建右子树再建左子树
            root->right = build(in_index + 1, in_end);
            root->left  = build(in_begin, in_index);
            return root;
          };
          return build(0, in_order.size());
        }
        ```

    !!! tip "优化"

        由于树中无重复元素，可以使用哈希表来加速查找根节点在中序遍历中的位置，从而优化时间复杂度。

??? note "恢复多叉树"

    给定一棵 $n$ 个节点的有根多叉树的前序和后序遍历，求其每一层的最右侧节点的编号。

    ??? hint 

        - 多叉树的前序遍历：先访问根节点，然后依次访问每个子树的前序遍历结果 $root \longrightarrow children(1..k)$
        - 多叉树的后序遍历：依次访问每个子树的后序遍历结果，最后访问根节点 $children(1..k) \longrightarrow root$

        仅凭这两种遍历无法唯一确定二叉树结构，但是对于每一层的最右侧节点编号是唯一的。不需要唯一的树结构，只要能得到"遍历的层次边界"，就能正确输出每层最右节点。

    !!! warning "NOT FULLY TESTED"

        该代码未经充分测试，仅供参考。

    ```cpp
    --8<-- "code/Tree/Traversal/reconstruct-nary-tree.cpp"
    ```

## Morris 遍历

$\text{Morris}$ 遍历是一种在不使用额外空间的情况下进行二叉树遍历的方法。它通过修改树的结构来实现遍历，具体步骤如下：

1. 初始化当前节点为根节点
2. 当当前节点不为空时，执行以下操作：
      1. 如果当前节点的左子节点为空，则访问当前节点并将当前节点移动到右子节点
      2. 如果当前节点的左子节点不为空，则找到当前节点左子树的最右节点（即左子树中最右的节点）
         1. 如果最右节点的右子节点为空，则将其右子节点指向当前节点，然后将当前节点移动到左子节点
         2. 如果最右节点的右子节点指向当前节点，则将其右子节点设为空，访问当前节点，然后将当前节点移动到右子节点
3. 重复步骤 2 直到当前节点为空

???+ note "$\text{Morris}$ 遍历"

    ??? info "二叉树的节点定义"

        ```cpp
        struct TreeNode {
          int val;
          TreeNode *left;
          TreeNode *right;

          TreeNode() : val(0), left(nullptr), right(nullptr) {}

          explicit TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}

          TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
        };
        ```

    === "Morris 遍历"

        ```cpp
        vector<int> morris_traversal(TreeNode *root) {
          vector<int> morris_traverse;
          TreeNode *current = root;
          while (current != nullptr) {
            morris_traverse.emplace_back(current->val);  // 收集morris遍历顺序
            if (current->left == nullptr) {              // 如果没有左子树，访问当前节点，转向右子树
              current = current->right;
              continue;
            }
            // 有左子树
            TreeNode *most_right = current->left;  // 找左子树的最右节点
            while (most_right->right != nullptr && most_right->right != current) {
              most_right = most_right->right;
            }
            if (most_right->right == nullptr) {  // 最右节点的右指针为空，建立线索，转向左子树
              most_right->right = current;
              current           = current->left;
            } else {  // 最右节点的右指针指向当前节点，说明左子树已经访问过了，断开线索，访问当前节点，转向右子树
              most_right->right = nullptr;
              current           = current->right;
            }
          }
          return morris_traverse;
        }
        ```

    === "前序遍历"

        ```cpp
        vector<int> morris_pre_order_traversal(TreeNode *root) {
          vector<int> traverse;
          TreeNode *current = root;
          while (current != nullptr) {
            if (current->left == nullptr) {         // 如果没有左子树，访问当前节点，转向右子树
              traverse.emplace_back(current->val);  // 第一次到达该节点时访问
              current = current->right;
              continue;
            }
            // 有左子树
            TreeNode *most_right = current->left;  // 找左子树的最右节点
            while (most_right->right != nullptr && most_right->right != current) {
              most_right = most_right->right;
            }
            if (most_right->right == nullptr) {     // 最右节点的右指针为空，建立线索，转向左子树
              traverse.emplace_back(current->val);  // 左子树仍未遍历, 第一次到达该节点时访问
              most_right->right = current;
              current           = current->left;
            } else {  // 最右节点的右指针指向当前节点，说明左子树已经访问过了，断开线索，转向右子树
              most_right->right = nullptr;
              current           = current->right;
            }
          }
          return traverse;
        }
        ```

    === "中序遍历"

        ```cpp
        vector<int> morris_in_order_traversal(TreeNode *root) {
          vector<int> traverse;
          TreeNode *current = root;
          while (current != nullptr) {
            if (current->left == nullptr) {         // 如果没有左子树，访问当前节点，转向右子树
              traverse.emplace_back(current->val);  // 访问当前节点
              current = current->right;
              continue;
            }
            // 有左子树
            TreeNode *most_right = current->left;  // 找左子树的最右节点
            while (most_right->right != nullptr && most_right->right != current) {
              most_right = most_right->right;
            }
            if (most_right->right == nullptr) {  // 最右节点的右指针为空，建立线索，转向左子树
              most_right->right = current;
              current           = current->left;
            } else {  // 最右节点的右指针指向当前节点，说明左子树已经访问过了，断开线索，访问当前节点，转向右子树
              most_right->right = nullptr;
              traverse.emplace_back(current->val);  // 左子树已遍历, 访问当前节点
              current = current->right;
            }
          }
          return traverse;
        }
        ```

    === "后序遍历"

        ```cpp
        vector<int> morris_post_order_traversal(TreeNode *root) {
          vector<int> traverse;
          TreeNode *current = root;

          // 逆序输出链表上的节点值
          auto reverse = [](TreeNode *from) {
            TreeNode *x = nullptr, *y = nullptr;
            while (from != nullptr) {
              y           = from->right;
              from->right = x;
              x           = from;
              from        = y;
            }
            return x;
          };

          // 逆序收集 from -> ... -> 这条链上的节点值
          auto collect = [&](TreeNode *from) {
            TreeNode *tail    = reverse(from);  // 反转链表
            TreeNode *current = tail;
            while (current != nullptr) {
              traverse.emplace_back(current->val);
              current = current->right;
            }
            reverse(tail);  // 恢复链表顺序
          };

          while (current != nullptr) {
            if (current->left == nullptr) {  // 如果没有左子树，访问当前节点，转向右子树
              current = current->right;
              continue;
            }
            // 有左子树
            TreeNode *most_right = current->left;  // 找左子树的最右节点
            while (most_right->right != nullptr && most_right->right != current) {
              most_right = most_right->right;
            }
            if (most_right->right == nullptr) {  // 最右节点的右指针为空，建立线索，转向左子树
              most_right->right = current;
              current           = current->left;
            } else {  // 最右节点的右指针指向当前节点，说明左子树已经访问过了，断开线索，访问当前节点，转向右子树
              most_right->right = nullptr;
              collect(current->left);  // 收集左子树的最右节点到当前节点的路径
              current = current->right;
            }
          }
          collect(root);  // 最后收集整棵树的右链
          return traverse;
        }
        ```

#include <functional>
using namespace std;

class TreeNode {
 public:
  int val;
  TreeNode *left;
  TreeNode *right;
};

class Solution {
 public:
  int largestBSTSubtree(TreeNode *root) {
    int max_size = 0;

    // 定义返回值: {是否为BST, 子树大小, 子树最小值, 子树最大值}
    struct Status {
      bool is_bst;
      int size;
      int min;
      int max;
    };

    function<Status(TreeNode *)> dfs = [&](TreeNode *node) -> Status {
      // 空节点视为BST, 大小为0, 最小值为+∞, 最大值为-∞
      if (!node) { return Status{.is_bst = true, .size = 0, .min = INT_MAX, .max = INT_MIN}; }
      auto left  = dfs(node->left);
      auto right = dfs(node->right);
      Status res;
      // 判断当前节点是否为BST
      if (left.is_bst && right.is_bst && node->val > left.max && node->val < right.min) {
        res.is_bst = true;                        // 是BST
        res.size   = left.size + right.size + 1;  // 子树大小
        res.min    = min(left.min, node->val);    // 子树最小值
        res.max    = max(right.max, node->val);   // 子树最大值
        max_size   = max(max_size, res.size);     // 更新最大BST子树大小
      } else {
        res.is_bst = false;                       // 不是BST
        res.size   = max(left.size, right.size);  // 最大BST子树大小
        // 最小值和最大值无意义
      }
      return res;
    };
    dfs(root);
    return max_size;
  }
};
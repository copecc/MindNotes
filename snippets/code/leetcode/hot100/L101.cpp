class Solution {
 private:
  bool mirror(TreeNode *a, TreeNode *b) {
    if (!a || !b) return a == b;
    return a->val == b->val && mirror(a->left, b->right) && mirror(a->right, b->left);
  }

 public:
  bool isSymmetric(TreeNode *root) {
    if (!root) return true;
    return mirror(root->left, root->right);
  }
};

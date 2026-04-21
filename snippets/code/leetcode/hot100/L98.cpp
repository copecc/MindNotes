#include <climits>
using namespace std;

class Solution {
 private:
  bool valid(TreeNode *root, long long lo, long long hi) {
    if (!root) return true;
    if (root->val <= lo || root->val >= hi) return false;
    return valid(root->left, lo, root->val) && valid(root->right, root->val, hi);
  }

 public:
  bool isValidBST(TreeNode *root) {
    return valid(root, LLONG_MIN, LLONG_MAX);
  }
};

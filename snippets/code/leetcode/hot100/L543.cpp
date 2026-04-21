#include <algorithm>
using namespace std;

class Solution {
 private:
  int depth(TreeNode *root, int &ans) {
    if (!root) return 0;
    int left = depth(root->left, ans);
    int right = depth(root->right, ans);
    ans = max(ans, left + right);
    return 1 + max(left, right);
  }

 public:
  int diameterOfBinaryTree(TreeNode *root) {
    int ans = 0;
    depth(root, ans);
    return ans;
  }
};

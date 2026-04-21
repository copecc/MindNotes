#include <vector>
using namespace std;

class Solution {
 private:
  TreeNode *build(const vector<int> &nums, int l, int r) {
    if (l > r) return nullptr;
    int mid = l + (r - l) / 2;
    TreeNode *root = new TreeNode(nums[mid]);
    root->left = build(nums, l, mid - 1);
    root->right = build(nums, mid + 1, r);
    return root;
  }

 public:
  TreeNode *sortedArrayToBST(vector<int> &nums) {
    return build(nums, 0, static_cast<int>(nums.size()) - 1);
  }
};

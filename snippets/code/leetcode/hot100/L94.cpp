#include <stack>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<int> inorderTraversal(TreeNode *root) {
    vector<int> ans;
    stack<TreeNode *> st;
    TreeNode *cur = root;
    while (cur || !st.empty()) {
      while (cur) {
        st.push(cur);
        cur = cur->left;
      }
      cur = st.top();
      st.pop();
      ans.push_back(cur->val);
      cur = cur->right;
    }
    return ans;
  }
};

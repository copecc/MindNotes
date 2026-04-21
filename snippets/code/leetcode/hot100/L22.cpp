#include <string>
#include <vector>
using namespace std;

class Solution {
 private:
  void dfs(int open, int close, int n, string &path, vector<string> &ans) {
    if (path.size() == static_cast<size_t>(2 * n)) {
      ans.push_back(path);
      return;
    }
    if (open < n) {
      path.push_back('(');
      dfs(open + 1, close, n, path, ans);
      path.pop_back();
    }
    if (close < open) {
      path.push_back(')');
      dfs(open, close + 1, n, path, ans);
      path.pop_back();
    }
  }

 public:
  vector<string> generateParenthesis(int n) {
    vector<string> ans;
    string path;
    dfs(0, 0, n, path, ans);
    return ans;
  }
};

#include <string>
#include <vector>
using namespace std;

class Solution {
 private:
  bool isPalindrome(const string &s, int l, int r) {
    while (l < r) {
      if (s[l++] != s[r--]) return false;
    }
    return true;
  }

  void dfs(int start, const string &s, vector<string> &path, vector<vector<string>> &ans) {
    if (start == s.size()) {
      ans.push_back(path);
      return;
    }
    for (int end = start; end < s.size(); ++end) {
      if (!isPalindrome(s, start, end)) continue;
      path.push_back(s.substr(start, end - start + 1));
      dfs(end + 1, s, path, ans);
      path.pop_back();
    }
  }

 public:
  vector<vector<string>> partition(string s) {
    vector<vector<string>> ans;
    vector<string> path;
    dfs(0, s, path, ans);
    return ans;
  }
};

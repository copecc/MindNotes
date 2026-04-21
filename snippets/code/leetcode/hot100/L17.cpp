#include <string>
#include <vector>
using namespace std;

class Solution {
 private:
  const vector<string> mapping = {
      "",    "",    "abc",  "def", "ghi",
      "jkl", "mno", "pqrs", "tuv", "wxyz"};

  void dfs(const string &digits, int idx, string &path, vector<string> &ans) {
    if (idx == digits.size()) {
      ans.push_back(path);
      return;
    }
    for (char ch : mapping[digits[idx] - '0']) {
      path.push_back(ch);
      dfs(digits, idx + 1, path, ans);
      path.pop_back();
    }
  }

 public:
  vector<string> letterCombinations(string digits) {
    vector<string> ans;
    if (digits.empty()) return ans;
    string path;
    dfs(digits, 0, path, ans);
    return ans;
  }
};

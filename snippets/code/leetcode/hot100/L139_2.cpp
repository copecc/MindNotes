#include <string>
#include <unordered_set>
#include <vector>
using namespace std;

class Solution {
 public:
  bool wordBreak(string s, vector<string>& wordDict) {
    unordered_set<string> dict(wordDict.begin(), wordDict.end());
    vector<int> memo(s.size(), -1);
    return dfs(s, 0, dict, memo);
  }

 private:
  bool dfs(const string& s, int start, const unordered_set<string>& dict,
           vector<int>& memo) {
    if (start == s.size()) {
      return true;
    }
    if (memo[start] != -1) {
      return memo[start];  // 同一个起点是否可拆分只需要判断一次。
    }
    for (int end = start + 1; end <= s.size(); ++end) {
      if (!dict.count(s.substr(start, end - start))) {
        continue;
      }
      if (dfs(s, end, dict, memo)) {
        memo[start] = 1;
        return true;
      }
    }
    memo[start] = 0;
    return false;
  }
};

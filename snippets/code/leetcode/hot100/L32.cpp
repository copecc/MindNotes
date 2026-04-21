#include <algorithm>
#include <string>
#include <vector>
using namespace std;

class Solution {
 public:
  int longestValidParentheses(string s) {
    int n = s.size();
    vector<int> dp(n, 0);
    int ans = 0;
    for (int i = 1; i < n; ++i) {
      if (s[i] != ')') {
        continue;
      }
      if (s[i - 1] == '(') {
        dp[i] = (i >= 2 ? dp[i - 2] : 0) + 2;
      } else {
        int pre = i - dp[i - 1] - 1;
        if (pre >= 0 && s[pre] == '(') {
          dp[i] = dp[i - 1] + 2;
          if (pre >= 1) {
            dp[i] += dp[pre - 1];
          }
        }
      }
      ans = max(ans, dp[i]);
    }
    return ans;
  }
};

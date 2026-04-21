#include <algorithm>
#include <climits>
#include <vector>
using namespace std;

class Solution {
 public:
  int numSquares(int n) {
    const int INF = INT_MAX / 2;
    vector<int> dp(n + 1, INF);
    dp[0] = 0;
    for (int sq = 1; sq * sq <= n; ++sq) {
      int coin = sq * sq;
      for (int j = coin; j <= n; ++j) {
        dp[j] = min(dp[j], dp[j - coin] + 1);
      }
    }
    return dp[n];
  }
};

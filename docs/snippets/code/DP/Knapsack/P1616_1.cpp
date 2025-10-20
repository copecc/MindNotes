#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int t, m;
  cin >> t >> m;
  vector<int> weights(m + 1), values(m + 1);
  for (int i = 1; i <= m; ++i) { cin >> weights[i] >> values[i]; }
  vector<vector<int64_t>> dp(m + 1, vector<int64_t>(t + 1, 0));
  for (int i = 1; i <= m; ++i) {
    for (int j = 1; j <= t; ++j) {
      dp[i][j] = dp[i - 1][j];
      if (j >= weights[i]) { dp[i][j] = max(dp[i][j], dp[i][j - weights[i]] + values[i]); }
    }
  }
  cout << dp[m][t] << '\n';
  return 0;
}
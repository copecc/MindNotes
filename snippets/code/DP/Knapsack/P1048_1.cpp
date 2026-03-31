#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int t, m;
  cin >> t >> m;
  vector<int> weights(m + 1), values(m + 1);
  for (int i = 1; i <= m; ++i) { cin >> weights[i] >> values[i]; }
  vector<vector<int>> dp(m + 1, vector<int>(t + 1, 0));
  for (int i = 1; i <= m; ++i) {
    for (int j = 1; j <= t; ++j) {
      if (j >= weights[i]) {  // 可以选择放入第 i 个物品或者不放入
        dp[i][j] = max(dp[i - 1][j], dp[i - 1][j - weights[i]] + values[i]);
      } else {  // 容量不足，只能选择不放入
        dp[i][j] = dp[i - 1][j];
      }
    }
  }
  cout << dp[m][t] << '\n';
  return 0;
}
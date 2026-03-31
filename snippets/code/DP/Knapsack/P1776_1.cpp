#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n, w;
  cin >> n >> w;
  vector<int> weights(n + 1), values(n + 1), counts(n + 1);
  for (int i = 1; i <= n; ++i) { cin >> values[i] >> weights[i] >> counts[i]; }
  vector<vector<int>> dp(n + 1, vector<int>(w + 1, 0));
  for (int i = 1; i <= n; ++i) {
    for (int j = 0; j <= w; ++j) {
      dp[i][j] = dp[i - 1][j];                                       // 不放入第i件物品
      for (int k = 1; k <= counts[i] && k * weights[i] <= j; ++k) {  // 放入k件第i件物品
        dp[i][j] = max(dp[i][j], dp[i - 1][j - k * weights[i]] + k * values[i]);
      }
    }
  }
  cout << dp[n][w] << '\n';
  return 0;
}
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
  vector<int64_t> dp(t + 1, 0);
  for (int i = 1; i <= m; ++i) {
    for (int j = weights[i]; j <= t; ++j) { dp[j] = max(dp[j], dp[j - weights[i]] + values[i]); }
  }
  cout << dp[t] << '\n';
  return 0;
}
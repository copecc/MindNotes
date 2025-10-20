#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int t, m;
  cin >> t >> m;
  vector<int> weights(m + 1), values(m + 1);
  for (int i = 1; i <= m; ++i) { cin >> weights[i] >> values[i]; }
  vector<int> dp(t + 1, 0);
  for (int i = 1; i <= m; ++i) {
    for (int j = t; j >= weights[i]; --j) {  // 逆序遍历, 保证 dp[j-weights[i]] 是上一行的结果
      dp[j] = max(dp[j], dp[j - weights[i]] + values[i]);
    }
  }
  cout << dp[t] << '\n';
  return 0;
}
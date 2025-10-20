#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n, w;
  cin >> n >> w;
  vector<int> weights(n + 1), values(n + 1), counts(n + 1);
  for (int i = 1; i <= n; ++i) { cin >> values[i] >> weights[i] >> counts[i]; }
  vector<int> dp(w + 1, 0);
  for (int i = 1; i <= n; ++i) {
    // 注意这里要逆序遍历, 保证 dp[j-k*weights[i]] 是上一行的结果
    for (int j = w; j >= weights[i]; --j) {
      for (int k = 1; k <= counts[i] && k * weights[i] <= j; ++k) {
        dp[j] = max(dp[j], dp[j - k * weights[i]] + k * values[i]);
      }
    }
  }
  cout << dp[w] << '\n';
  return 0;
}
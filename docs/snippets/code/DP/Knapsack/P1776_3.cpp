#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n, w;
  cin >> n >> w;
  vector<int> weights(n + 1), values(n + 1), counts(n + 1);
  for (int i = 1; i <= n; i++) { cin >> values[i] >> weights[i] >> counts[i]; }
  vector<int> dp(w + 1, 0);
  for (int i = 0; i < weights.size(); ++i) {
    int count = counts[i];
    for (int k = 1; count > 0; k <<= 1) {  // 将count拆分为若干个2的幂次方之和
      int use     = min(k, count);         // 不足2的幂次方则全部使用, 则单独作为一组
      count      -= use;
      int weight  = use * weights[i];
      int value   = use * values[i];
      for (int j = w; j >= weight; --j) { dp[j] = max(dp[j], dp[j - weight] + value); }
    }
  }
  cout << dp[w] << '\n';
  return 0;
}
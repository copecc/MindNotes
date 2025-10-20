#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int k, v, n;
  cin >> k >> v >> n;
  vector<int> weights(n + 1);
  vector<int> values(n + 1);
  for (int i = 1; i <= n; i++) { cin >> weights[i] >> values[i]; }

  using VI   = vector<int>;
  using VVI  = vector<VI>;
  using VVVI = vector<VVI>;
  VVVI dp    = VVVI(n + 1, VVI(v + 1, VI(k, INT32_MIN)));
  for (int i = 0; i <= n; i++) { dp[i][0][0] = 0; }

  for (int i = 1; i <= n; ++i) {
    for (int j = 0; j <= v; ++j) {
      vector<int> not_chosen = dp[i - 1][j];  // 不选第i个物品
      if (j >= weights[i]) {                  // 选第i个物品, 需要加上物品的价值
        vector<int> chosen = dp[i - 1][j - weights[i]];
        for (int l = 0; l < k; l++) { chosen[l] += values[i]; }
        not_chosen.insert(not_chosen.end(), chosen.begin(), chosen.end());
      }
      // 取前 k 大的解
      sort(not_chosen.begin(), not_chosen.end(), greater<>());
      not_chosen.resize(k);
      dp[i][j] = not_chosen;
    }
  }

  // 计算前 k 个最优解的总价值
  int ans = 0;
  for (int i = 0; i < k; i++) { ans += dp[n][v][i]; }

  cout << ans << '\n';
  return 0;
}
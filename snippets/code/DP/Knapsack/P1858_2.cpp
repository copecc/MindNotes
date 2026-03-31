#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int k, v, n;
  cin >> k >> v >> n;
  vector<int> weights(n + 1);
  vector<int> values(n + 1);
  for (int i = 1; i <= n; i++) { cin >> weights[i] >> values[i]; }
  vector<vector<int>> dp(v + 1, vector<int>(k, INT32_MIN));
  dp[0][0] = 0;

  for (int i = 1; i <= n; i++) {
    for (int j = v; j >= weights[i]; j--) {
      vector<int> now(k, 0);           // 保存当前状态的解
      int not_choose = 0, choose = 0;  // 指向两个转移状态的第k大的解
      for (int l = 0; l < k; ++l) {
        if (dp[j][not_choose] > dp[j - weights[i]][choose] + values[i]) {  // 合并排序
          now[l] = dp[j][not_choose];
          not_choose++;
        } else {
          now[l] = dp[j - weights[i]][choose] + values[i];
          choose++;
        }
      }
      dp[j] = now;
    }
  }

  // 计算前 k 个最优解的总价值
  int ans = 0;
  for (int i = 0; i < k; i++) { ans += dp[v][i]; }

  cout << ans << '\n';
  return 0;
}
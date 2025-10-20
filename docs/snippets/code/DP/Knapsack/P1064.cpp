#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n, m;
  cin >> n >> m;
  vector<int> weights(m + 1), values(m + 1), parents(m + 1);
  vector<int> kings(m + 1);               // kings[i]表示物品i是否为主件物品
  vector<vector<int>> dependents(m + 1);  // dependents[i]表示物品i的附属物品
  for (int i = 1; i <= m; ++i) {
    cin >> weights[i] >> values[i] >> parents[i];
    values[i] *= weights[i];  // 转化为价值背包
    if (parents[i] == 0) {    // 物品i为主件物品
      kings[i] = 1;
    } else {  // 物品i为物品parents[i]的附属物品
      dependents[parents[i]].push_back(i);
    }
  }

  vector<int> dp(n + 1, 0);
  for (int i = 1; i <= m; ++i) {
    // 只处理主件物品
    if (kings[i] == 0) { continue; }
    // 枚举主件物品i的所有可能的组合(包括不选任何附属物品)
    for (int j = n; j >= weights[i]; --j) {
      // 先不选附属物品, 只考虑主件物品
      dp[j] = max(dp[j], dp[j - weights[i]] + values[i]);
      // 枚举附属物品的所有组合, 此处假设附属不超过32件
      int m = dependents[i].size();
      for (int s = 1; s < (1 << m); ++s) {
        int total_weight = weights[i];  // 主件物品i的重量, 必须选
        int total_value  = values[i];   // 主件物品i的价值, 必须选
        for (int k = 0; k < m; ++k) {
          if ((s & (1 << k)) != 0) {  // 选择附属物品k
            total_weight += weights[dependents[i][k]];
            total_value  += values[dependents[i][k]];
          }
        }
        if (total_weight <= j) { dp[j] = max(dp[j], dp[j - total_weight] + total_value); }
      }
    }
  }
  cout << dp[n] << '\n';

  return 0;
}
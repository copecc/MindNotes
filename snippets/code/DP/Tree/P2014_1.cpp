#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n, m;
  cin >> n >> m;
  vector<vector<int>> tree(n + 1);
  vector<int> weights(n + 1, 1);
  vector<int> values(n + 1);
  for (int i = 1; i <= n; i++) {
    int k, s;
    cin >> k >> s;
    tree[k].push_back(i);
    values[i] = s;
  }
  weights[0] = values[0] = 0;  // 根节点重量和价值为0

  vector<vector<int>> dp(n + 1, vector<int>(m + 1));
  function<void(int, int)> dfs = [&](int u, int from) {
    // 初始化: 只放入当前节点
    for (int j = weights[u]; j <= m; ++j) { dp[u][j] = values[u]; }
    for (int v : tree[u]) {
      if (v == from) { continue; }
      dfs(v, u);
      // 将子节点的物品合并到当前节点的背包中, 01背包倒序枚举，完全背包正序枚举
      for (int j = m; j >= weights[u]; --j) {        // 当前背包容量
        for (int k = 0; k <= j - weights[u]; ++k) {  // 分配给当前子树的容量
          dp[u][j] = max(dp[u][j], dp[u][j - k] + dp[v][k]);
        }
      }
    }
  };
  dfs(0, -1);
  cout << dp[0][m] << '\n';

  return 0;
}
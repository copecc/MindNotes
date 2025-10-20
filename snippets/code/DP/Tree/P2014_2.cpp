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
  vector<int> size(n + 1, 0);  // size[u]: u及其子树的总重量

  function<void(int, int)> dfs = [&](int u, int from) {
    size[u] = weights[u];
    for (int j = weights[u]; j <= m; ++j) { dp[u][j] = values[u]; }
    for (int v : tree[u]) {
      if (v == from) { continue; }
      dfs(v, u);
      int upper_j = min(size[u] + size[v], m);
      for (int j = upper_j; j >= weights[u]; --j) {  // 当前背包容量
        for (int k = max(1, j - size[u]); k <= min(size[v], j - weights[u]); ++k) {
          dp[u][j] = max(dp[u][j], dp[u][j - k] + dp[v][k]);
        }
      }
      size[u] += size[v];
    }
  };
  dfs(0, -1);
  cout << dp[0][min(size[0], m)] << "\n";

  return 0;
}
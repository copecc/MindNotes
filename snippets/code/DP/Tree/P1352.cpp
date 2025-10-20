#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n;
  cin >> n;
  vector<vector<int>> tree(n + 1);
  vector<int> happiness(n + 1);
  vector<int> is_root(n + 1, 1);
  // dp[i][0]: i不被子节点覆盖的最大快乐值
  // dp[i][1]: i被子节点覆盖的最大快乐值
  vector<vector<int>> dp(n + 1, vector<int>(2));
  for (int i = 1; i <= n; i++) { cin >> happiness[i]; }
  for (int i = 1; i < n; i++) {
    int l, k;
    cin >> l >> k;
    tree[k].push_back(l);
    is_root[l] = 0;
  }

  function<void(int, int)> dfs = [&](int u, int from) {
    dp[u][0] = 0;
    dp[u][1] = happiness[u];
    for (int v : tree[u]) {
      if (v == from) { continue; }
      dfs(v, u);
      dp[u][0] += max(dp[v][0], dp[v][1]);  // 选择不来，所以子节点可以选择来或不来
      dp[u][1] += dp[v][0];                 // 选择来，所以子节点必须选择不来
    }
  };

  int root = find(is_root.begin() + 1, is_root.end(), 1) - is_root.begin();
  dfs(root, 0);

  cout << max(dp[root][0], dp[root][1]) << '\n';
  return 0;
}
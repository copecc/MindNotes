#include <algorithm>
#include <climits>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n;
  cin >> n;
  vector<vector<int>> tree(n + 1);
  vector<vector<int>> dp(n + 1, vector<int>(3));
  for (int i = 1; i < n; i++) {
    int x, y;
    cin >> x >> y;
    tree[x].push_back(y);
    tree[y].push_back(x);
  }

  auto metric = [&](int i) { return dp[i][0] - min(dp[i][0], dp[i][1]); };

  auto dfs    = [&](auto &&self, int u, int from) -> void {
    dp[u][0]       = 1;   // 被自己覆盖
    dp[u][1]       = 0;   // 被子节点覆盖
    dp[u][2]       = 0;   // 被父节点覆盖
    int best_child = -1;  // 选择哪个子节点覆盖自己

    for (int v : tree[u]) {
      if (v == from) { continue; }
      self(self, v, u);
      // 节点被自己覆盖, 则子节点可以选择被自己覆盖或者被父节点覆盖,
      // 或者被自己的子节点覆盖
      dp[u][0] += min({dp[v][0], dp[v][1], dp[v][2]});
      // 节点被子节点覆盖, 则必须选择一个最优的子节点覆盖自己
      dp[u][1] += min(dp[v][0], dp[v][1]);
      if (best_child == -1 || metric(v) < metric(best_child)) { best_child = v; }
      // 节点被父节点覆盖, 则子节点必须选择被自己覆盖或者被子节点覆盖
      dp[u][2] += min(dp[v][0], dp[v][1]);
    }
    // 公式最后的差值。如果没有子节点，则无法被子节点覆盖，因此设为无穷大
    dp[u][1] = (best_child == -1 ? INT_MAX : dp[u][1] + metric(best_child));
  };
  dfs(dfs, 1, 0);
  // 根节点没有父节点, 所以不能被父节点覆盖
  cout << min(dp[1][0], dp[1][1]) << '\n';
  return 0;
}
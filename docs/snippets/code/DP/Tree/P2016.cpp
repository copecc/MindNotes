#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n;
  cin >> n;
  vector<vector<int>> tree(n);
  vector<int> is_root(n, 1);
  vector<vector<int>> dp(n, vector<int>(2));
  for (int i = 0; i < n; i++) {
    int x, k;
    cin >> x >> k;
    for (int j = 0; j < k; j++) {
      int y;
      cin >> y;
      tree[x].push_back(y);
      is_root[y] = 0;
    }
  }

  function<void(int, int)> dfs = [&](int u, int from) {
    dp[u][0] = 1;
    dp[u][1] = 0;
    for (int v : tree[u]) {
      if (v == from) { continue; }
      dfs(v, u);
      dp[u][0] += dp[v][1];                 // u不选，所以子节点必须选择
      dp[u][1] += min(dp[v][0], dp[v][1]);  // u选, 所以子节点可选可不选
    }
  };
  int root = find(is_root.begin(), is_root.end(), 1) - is_root.begin();
  dfs(root, -1);
  cout << min(dp[root][0], dp[root][1]) << '\n';
  return 0;
}
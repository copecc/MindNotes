#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

void solve(int n, int m, int t) {
  vector<vector<int>> g(n + 1);
  for (int i = 0; i < m; ++i) {
    int u, v;
    cin >> u >> v;
    g[u].push_back(v);
    g[v].push_back(u);
  }

  vector<int> dfn(n + 1);  // 遍历到该点的时间戳 depth first number
  vector<int> low(n + 1);  // 从该点出发所能到达的最小时间戳
  vector<int> bridge(n + 1);
  int bridge_count = 0;

  vector<int> parent(n + 1);
  vector<int> depth(n + 1);

  int timer   = 0;
  auto tarjan = [&](auto &&self, int u, int from) -> void {
    parent[u] = from;
    dfn[u] = low[u] = ++timer;
    for (int v : g[u]) {
      if (!dfn[v]) {
        depth[v] = depth[u] + 1;
        self(self, v, u);
        if (low[v] > dfn[u]) {
          bridge[v] = u;   // 标记该边为桥
          ++bridge_count;  // 桥数量加一
        }
        low[u] = min(low[u], low[v]);
      } else if (v != from) {
        low[u] = min(low[u], dfn[v]);
      }
    }
  };

  for (int i = 0; i < n; i++) {
    if (dfn[i] == 0) { tarjan(tarjan, i, -1); }
  }
  // 连边后形成环, 环上的桥都不再是桥
  auto get_lca = [&](int x, int y) {
    if (depth[x] > depth[y]) { swap(x, y); }  // 确保x深度更小
    while (depth[y] > depth[x]) {             // 提升y到和x同一深度
      if (bridge[y] != 0) {                   // 该边不再是桥
        bridge_count--;
        bridge[y] = 0;
      }
      y = parent[y];
    }
    if (y == x) { return; }  // 已经是同一个祖先
    while (x != y) {
      if (bridge[x] != 0) {
        bridge_count--;
        bridge[x] = 0;
      }
      if (bridge[y] != 0) {
        bridge_count--;
        bridge[y] = 0;
      }
      x = parent[x];
      y = parent[y];
    }
  };

  cout << "Case " << t << ":\n";
  int q;
  cin >> q;
  for (int i = 0; i < q; ++i) {
    int u, v;
    cin >> u >> v;
    get_lca(u, v);
    cout << bridge_count << "\n";
  }
  cout << "\n";
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int t = 1;
  int n, m;
  while (cin >> n >> m) {
    if (n == 0 && m == 0) { break; }
    solve(n, m, t);
    t++;
  }
  return 0;
}
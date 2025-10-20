#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

void solve() {}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n, m;
  cin >> n >> m;

  vector<vector<int>> g(n + 1);
  for (int i = 0; i < m; ++i) {
    int u, v;
    cin >> u >> v;
    g[u].push_back(v);
    g[v].push_back(u);
  }

  vector<int> dfn(n + 1);  // 遍历到该点的时间戳 depth first number
  vector<int> low(n + 1);  // 从该点出发所能到达的最小时间戳
  vector<int> articulation;

  int timer   = 0;
  auto tarjan = [&](auto &&self, int u, int from) -> void {
    dfn[u] = low[u] = ++timer;
    int child       = 0;
    bool valid      = false;
    for (int v : g[u]) {
      if (!dfn[v]) {
        ++child;
        self(self, v, u);
        if (low[v] >= dfn[u] && from != -1) { valid = true; }
        low[u] = min(low[u], low[v]);
      } else if (v != from) {
        low[u] = min(low[u], dfn[v]);
      }
    }
    if (valid || from == -1 && child >= 2) { articulation.push_back(u); }
  };
  for (int i = 1; i <= n; i++) {
    if (dfn[i] == 0) { tarjan(tarjan, i, -1); }
  }
  sort(articulation.begin(), articulation.end());
  cout << articulation.size() << "\n";
  for (int u : articulation) { cout << u << " "; }
  cout << "\n";
  return 0;
}
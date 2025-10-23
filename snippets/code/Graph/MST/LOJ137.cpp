#include <algorithm>
#include <iostream>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;

int A, B, C, P;

inline int rnd() { return A = (A * B + C) % P; };

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n, m;
  cin >> n >> m;
  using TIII = tuple<int, int, int>;
  vector<int> node(2 * n);  // 最多有 n-1 个新节点
  vector<vector<int>> tree(2 * n);
  vector<TIII> edges(m);
  for (int i = 0; i < m; ++i) {
    int u, v, w;
    cin >> u >> v >> w;
    edges[i] = {w, u, v};
  }

  vector<int> root(2 * n);
  iota(root.begin(), root.end(), 0);
  auto find = [&](int x) {
    while (root[x] != x) {
      root[x] = root[root[x]];
      x       = root[x];
    }
    return x;
  };

  // Kruskal重构
  sort(edges.begin(), edges.end());
  int idx = n, edge_count = 0;
  for (const auto &[w, u, v] : edges) {
    int ru = find(u);
    int rv = find(v);
    if (ru != rv) {
      ++idx;
      root[ru] = root[rv] = idx;
      node[idx]           = w;
      tree[idx].emplace_back(ru);
      tree[idx].emplace_back(rv);
      ++edge_count;
    }
    if (edge_count == n - 1) { break; }
  }

  int q;
  cin >> q >> A >> B >> C >> P;
  // Tarjan离线LCA
  vector<vector<pair<int, int>>> query(idx + 1);
  for (int i = 0; i < q; ++i) {
    int x = rnd() % n + 1;
    int y = rnd() % n + 1;
    query[x].emplace_back(y, i);
    query[y].emplace_back(x, i);
  }
  vector<bool> visited(2 * n);
  vector<int> answer(q);
  iota(root.begin(), root.end(), 0);
  auto dfs = [&](auto &&self, int x, int from) -> void {
    visited[x] = true;
    for (int y : tree[x]) {
      if (y != from) {
        self(self, y, x);
        root[y] = x;  // 合并 y 和 x
      }
    }
    for (auto [y, idx] : query[x]) {              // 处理所有和 x 有关的查询
      if (visited[y]) { answer[idx] = find(y); }  // y 已经访问过, 说明 LCA 已经确定
    }
  };
  dfs(dfs, idx, -1); // 从根节点开始DFS

  const int mod = 1e9 + 7;
  int ans       = 0;
  for (int lca : answer) { ans = (ans + node[lca]) % mod; }
  cout << ans << "\n";

  return 0;
}
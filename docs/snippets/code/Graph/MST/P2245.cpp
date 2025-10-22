#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, m;
  cin >> n >> m;
  using TIII = tuple<int64_t, int64_t, int64_t>;
  vector<int64_t> node(2 * n);  // 最多有 n-1 个新节点, 总节点数不超过 2n-1
  vector<vector<int64_t>> tree(2 * n);
  vector<TIII> edges(m);
  for (int64_t i = 0; i < m; ++i) {
    int64_t u, v, w;
    cin >> u >> v >> w;
    edges[i] = {w, u, v};
  }

  vector<int64_t> root(2 * n);
  iota(root.begin(), root.end(), 0);
  auto find = [&](int64_t x) {
    while (root[x] != x) {
      root[x] = root[root[x]];
      x       = root[x];
    }
    return x;
  };

  // Kruskal重构
  sort(edges.begin(), edges.end());
  int64_t idx = n;
  for (const auto &[w, u, v] : edges) {
    int64_t ru = find(u);
    int64_t rv = find(v);
    if (ru != rv) {
      root[ru] = root[rv] = ++idx;
      node[idx]           = w;
      tree[idx].emplace_back(ru);
      tree[idx].emplace_back(rv);
    }
  }

  // LCA预处理
  vector<int64_t> depth(idx + 1, 0);
  int64_t M = 32 - __builtin_clz(idx + 1);
  vector<vector<int64_t>> st(idx + 1, vector<int64_t>(M + 1, -1));
  auto dfs = [&](auto &&self, int64_t u, int64_t p) -> void {
    st[u][0] = p;
    for (int64_t i = 1; i <= M; ++i) {
      if (st[u][i - 1] != -1) { st[u][i] = st[st[u][i - 1]][i - 1]; }
    }
    for (const auto &v : tree[u]) {
      if (v != p) {
        depth[v] = depth[u] + 1;
        self(self, v, u);
      }
    }
  };
  for (int64_t i = 1; i <= idx; ++i) {      // 图可能不连通
    if (root[i] == i) { dfs(dfs, i, -1); }  // 从每个连通块的根节点开始dfs
  }

  auto get_kth_ancestor = [&](int node, int k) -> int {
    for (; (k != 0) && (node != -1); k &= k - 1) { node = st[node][__builtin_ctz(k)]; }
    return node;
  };

  auto get_lca = [&](int x, int y) -> int {
    if (depth[x] > depth[y]) { swap(x, y); }
    y = get_kth_ancestor(y, depth[y] - depth[x]);
    if (y == x) { return x; }
    for (int i = M - 1; i >= 0; --i) {
      int px = st[x][i];
      int py = st[y][i];
      if (px != py) {
        x = px;
        y = py;
      }
    }
    return st[x][0];
  };

  int64_t q;
  cin >> q;
  for (int64_t i = 0; i < q; ++i) {
    int64_t x, y;
    cin >> x >> y;
    if (find(x) != find(y)) {
      cout << "impossible\n";
    } else {
      int64_t lca = get_lca(x, y);
      cout << node[lca] << "\n";
    }
  }

  return 0;
}
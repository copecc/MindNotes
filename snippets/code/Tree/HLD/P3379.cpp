#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n, m, s;
  cin >> n >> m >> s;
  vector<vector<int>> tree(n + 1);
  for (int i = 0; i < n - 1; ++i) {
    int x, y;
    cin >> x >> y;
    tree[x].push_back(y);
    tree[y].push_back(x);
  }

  // 父节点, 深度, 子树大小, 重儿子
  vector<int64_t> parent(n + 1), depth(n + 1), size(n + 1), heavy_son(n + 1, -1);
  auto dfs = [&](auto &self, int64_t u, int64_t from) -> void {
    parent[u]        = from;
    size[u]          = 1;
    int64_t max_size = 0;
    for (int64_t v : tree[u]) {
      if (v != from) {
        depth[v] = depth[u] + 1;
        self(self, v, u);
        size[u] += size[v];
        if (size[v] > max_size) {
          max_size     = size[v];
          heavy_son[u] = v;
        }
      }
    }
  };
  dfs(dfs, s, -1);  // 计算父节点, 深度, 子树大小, 重儿子

  // 链顶, dfs序, dfn反序(dfn为i的节点编号)
  vector<int64_t> top(n + 1), dfn(n + 1), rank(n + 1);
  int64_t timer  = 0;
  auto decompose = [&](auto &self, int64_t u, int64_t t) -> void {
    top[u]      = t;
    dfn[u]      = ++timer;
    rank[timer] = u;
    if (heavy_son[u] != -1) { self(self, heavy_son[u], t); }
    for (int64_t v : tree[u]) {
      if (v != parent[u] && v != heavy_son[u]) { self(self, v, v); }
    }
  };
  decompose(decompose, s, s);  // 重链剖分

  auto get_lca = [&](int64_t u, int64_t v) -> int64_t {
    while (top[u] != top[v]) {
      if (depth[top[u]] < depth[top[v]]) { swap(u, v); }
      u = parent[top[u]];
    }
    return depth[u] < depth[v] ? u : v;
  };

  for (int i = 0; i < m; ++i) {
    int x, y;
    cin >> x >> y;
    cout << get_lca(x, y) << "\n";
  }

  return 0;
}
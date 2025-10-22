#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n;
  cin >> n;

  vector<vector<int64_t>> tree(n + 1);
  for (int64_t i = 1; i < n; ++i) {
    int64_t u, v;
    cin >> u >> v;
    tree[u].emplace_back(v);
    tree[v].emplace_back(u);
  }

  vector<int64_t> color(n + 1);
  for (int64_t i = 1; i <= n; ++i) { cin >> color[i]; }

  int64_t r = 1;  // 根节点
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
  dfs(dfs, r, -1);  // 计算父节点, 深度, 子树大小, 重儿子

  // 颜色计数器, 答案数组
  vector<int64_t> counter(n + 1), answer(n + 1);
  int64_t unique_colors = 0;

  // 树上启发式合并
  auto effect = [&](auto &&self, int64_t u) -> void {
    if (++counter[color[u]] == 1) { unique_colors++; }
    for (int64_t v : tree[u]) {
      if (v != parent[u]) { self(self, v); }
    }
  };

  auto deffect = [&](auto &&self, int64_t u) -> void {
    if (--counter[color[u]] == 0) { unique_colors--; }
    for (int64_t v : tree[u]) {
      if (v != parent[u]) { self(self, v); }
    }
  };

  auto dsu = [&](auto &self, int64_t u, bool keep) -> void {
    // 处理轻儿子
    for (int64_t v : tree[u]) {
      if (v != parent[u] && v != heavy_son[u]) { self(self, v, false); }
    }
    // 处理重儿子
    if (heavy_son[u] != -1) { self(self, heavy_son[u], true); }
    // 当前节点信息
    if (++counter[color[u]] == 1) { unique_colors++; }
    // 合并轻儿子的信息
    for (int64_t v : tree[u]) {
      if (v != parent[u] && v != heavy_son[u]) { effect(effect, v); }
    }
    answer[u] = unique_colors;
    // 如果不保留信息, 则清空当前子树的信息
    if (!keep) { deffect(deffect, u); }
  };
  dsu(dsu, r, false);

  int64_t m;
  cin >> m;
  for (int64_t i = 0; i < m; ++i) {
    int64_t u;
    cin >> u;
    cout << answer[u] << "\n";
  }
}
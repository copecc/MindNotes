#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  
  int64_t n;
  cin >> n;
  vector<pair<int64_t, int64_t>> balls(n + 1);
  for (int64_t i = 1; i <= n; i++) { cin >> balls[i].first >> balls[i].second; }
  vector<vector<int64_t>> tree(n + 1);
  for (int64_t i = 1; i < n; i++) {
    int64_t u, v;
    cin >> u >> v;
    tree[u].emplace_back(v);
    tree[v].emplace_back(u);
  }

  vector<int64_t> root(n + 1, 0);
  iota(root.begin(), root.end(), 0);
  vector<int64_t> size(n + 1, 1);        // 记录每个集合的大小
  vector<int64_t> edge_count(n + 1, 0);  // 记录每个集合中的边数
  vector<pair<int64_t, int64_t>> rollback;

  auto find = [&](int64_t x) {
    while (root[x] != x) { x = root[x]; }
    return x;
  };

  auto union_set = [&](int64_t x, int64_t y) {
    int64_t fx = find(x);
    int64_t fy = find(y);
    if (fx == fy) { return; }
    if (size[fx] < size[fy]) { swap(fx, fy); }
    root[fy]        = fx;
    size[fx]       += size[fy];
    edge_count[fx] += edge_count[fy] + 1;
    rollback.emplace_back(fy, fx);
  };

  auto union_rollback = [&]() {
    if (rollback.empty()) { return; }
    auto [fy, fx] = rollback.back();
    rollback.pop_back();
    root[fy]        = fy;
    size[fx]       -= size[fy];
    edge_count[fx] -= edge_count[fy] + 1;
  };

  int64_t count = 0;
  vector<int64_t> answer(n + 1, 0);
  auto dfs = [&](auto &self, int64_t u, int64_t from) -> void {
    int64_t fx = find(balls[u].first);
    int64_t fy = find(balls[u].second);
    bool added = false, merged = false;
    if (fx == fy) {  // 已经在同一集合中
      if (edge_count[fx] < size[fx]) {
        count++;  // 集合中边数小于节点数，可以多选一种球
        added = true;
      }
      edge_count[fx]++;
    } else {  // 不在同一集合中，合并
      if (edge_count[fx] < size[fx] || edge_count[fy] < size[fy]) {
        count++;  // 任一集合中边数小于节点数，可以多选一种球
        added = true;
      }
      union_set(fx, fy);
      merged = true;
    }
    answer[u] = count;
    for (auto &v : tree[u]) {
      if (v == from) { continue; }
      self(self, v, u);
    }
    // 回滚
    if (added) { count--; }
    if (merged) {
      union_rollback();
    } else {
      edge_count[fx]--;
    }
  };
  dfs(dfs, 1, -1);

  for (int64_t i = 2; i <= n; i++) { cout << answer[i] << " "; }
  cout << "\n";

  return 0;
}
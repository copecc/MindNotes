#include <algorithm>
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
  int n, m;
  cin >> n >> m;

  using TIII = tuple<int, int, int>;

  vector<TIII> queries(m + 1);
  int max_w = 0;
  for (int i = 1; i <= m; ++i) {
    int u, v, w;
    cin >> u >> v >> w;
    queries[i] = {u, v, w};
    max_w      = max(max_w, w);
  }
  max_w++;  // 有可能所有从0到max_w的权值都存在, 结果为 max_w+1

  // 根据权值轴建立线段树
  vector<vector<pair<int, int>>> interval(4 * max_w);
  auto add = [&](auto &self, int ql, int qr, int qx, int qy, int i, int l, int r) -> void {
    if (ql <= l && r <= qr) {
      interval[i].emplace_back(qx, qy);
      return;
    }
    int mid = (l + r) >> 1;
    if (ql <= mid) { self(self, ql, qr, qx, qy, 2 * i, l, mid); }
    if (qr > mid) { self(self, ql, qr, qx, qy, 2 * i + 1, mid + 1, r); }
  };
  // 权值为w的边，在[0, w-1]和[w+1, max_w]区间内存在
  for (auto [u, v, w] : queries) {
    if (w > 0) { add(add, 0, w - 1, u, v, 1, 0, max_w); }
    add(add, w + 1, max_w, u, v, 1, 0, max_w);
  }
  // 并查集，支持回滚
  int part = n;  // 连通块数量
  vector<int> root(n + 1);
  vector<int> size(n + 1, 1);
  vector<pair<int, int>> rollback;
  iota(root.begin(), root.end(), 0);
  auto find = [&](int x) {
    while (root[x] != x) { x = root[x]; }
    return x;
  };
  auto union_set = [&](int x, int y) {
    int fx = find(x);
    int fy = find(y);
    if (fx == fy) { return 0; }
    if (size[fx] < size[fy]) { swap(fx, fy); }
    root[fy]  = fx;
    size[fx] += size[fy];
    rollback.emplace_back(fy, fx);
    part--;  // 连通块数量减少
    return 1;
  };
  auto union_rollback = [&]() {
    if (rollback.empty()) { return; }
    auto [fy, fx] = rollback.back();
    rollback.pop_back();
    root[fy]  = fy;
    size[fx] -= size[fy];
    part++;  // 连通块数量增加
  };

  auto dfs = [&](auto &self, int i, int l, int r) -> int {
    int union_count = 0;
    for (auto &[u, v] : interval[i]) { union_count += union_set(u, v); }
    int res = -1;
    if (l == r) {
      if (part == 1) { res = l; } // 连通块数量为1，形成一棵生成树
    } else {
      int mid = (l + r) >> 1;
      res     = self(self, 2 * i, l, mid); // 先搜索左子树, 如果没找到再搜索右子树
      if (res == -1) { res = self(self, 2 * i + 1, mid + 1, r); }
    }
    for (int k = 0; k < union_count; ++k) { union_rollback(); }
    return res;
  };
  cout << dfs(dfs, 1, 0, max_w) << "\n";

  return 0;
}
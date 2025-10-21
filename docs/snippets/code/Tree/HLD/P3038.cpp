#include <algorithm>
#include <cstdint>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;

using PII  = pair<int, int>;
using TIII = tuple<int, int, int>;

struct segment_tree {
  vector<int64_t> sum;      // 区间和
  vector<int64_t> tag_add;  // 区间加法懒标记

  explicit segment_tree(int64_t n) : sum(n * 4), tag_add(n * 4) {}

  void push_up(int64_t i) { sum[i] = sum[2 * i] + sum[2 * i + 1]; }

  void lazy_add(int64_t i, int64_t val, int64_t count) {
    sum[i]     += count * val;
    tag_add[i] += val;
  }

  // 向下传递懒标记
  void push_down(int64_t i, int64_t left_count, int64_t right_count) {
    if (tag_add[i] != 0) {  // 将加法标记传递给子节点
      lazy_add(2 * i, tag_add[i], left_count);
      lazy_add(2 * i + 1, tag_add[i], right_count);
      tag_add[i] = 0;  // 清空根节点加法标记
    }
  }

  // 区间加法: range_add(x, y, val, 1, 1, n) 将区间 [x,y] 的值加上 val
  void range_add(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) {  // 区间覆盖, 直接更新
      lazy_add(i, val, r - l + 1);
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    if (ql <= mid) { range_add(ql, qr, val, 2 * i, l, mid); }
    if (qr > mid) { range_add(ql, qr, val, 2 * i + 1, mid + 1, r); }
    push_up(i);
  }

  // 区间求和: range_sum(x, y, 1, 1, n) 查询区间 [x,y] 的和
  int64_t range_sum(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return sum[i]; }  // 区间覆盖，直接返回
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    // 汇总结果
    int64_t res = 0;
    if (ql <= mid) { res += range_sum(ql, qr, 2 * i, l, mid); }
    if (qr > mid) { res += range_sum(ql, qr, 2 * i + 1, mid + 1, r); }
    return res;
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n, m;
  cin >> n >> m;

  vector<vector<int>> tree(n + 1);
  for (int i = 1; i < n; ++i) {
    int u, v;
    cin >> u >> v;
    tree[u].push_back(v);
    tree[v].push_back(u);
  }
  // 重链剖分准备
  int r = 1;  // 以1号节点为根节点
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

  // 链顶, dfs序, dfs反序(dfn为i的节点编号)
  vector<int64_t> top(n + 1), dfn(n + 1), rank(n + 1);
  int64_t timer  = 0;
  auto decompose = [&](auto &self, int64_t u, int64_t t) -> void {
    top[u]      = t;
    dfn[u]      = ++timer;
    rank[timer] = u;
    if (heavy_son[u] != -1) { self(self, heavy_son[u], t); }
    for (int64_t v : tree[u]) {  // 处理轻儿子, 轻儿子各自成链，链顶为自己
      if (v != parent[u] && v != heavy_son[u]) { self(self, v, v); }
    }
  };
  decompose(decompose, r, r);  // 重链剖分, 根节点为r, 链顶为自己
  // 构建线段树, 一开始都是关键边, 所以点权都为1
  segment_tree seg(n);
  // 查询u-v路径上的边权和
  auto path_sum = [&](int64_t u, int64_t v) {
    int64_t res = 0;
    while (top[u] != top[v]) {
      if (depth[top[u]] < depth[top[v]]) { swap(u, v); }
      res = (res + seg.range_sum(dfn[top[u]], dfn[u], 1, 1, n));
      u   = parent[top[u]];
    }
    if (depth[u] > depth[v]) { swap(u, v); }
    res = (res + seg.range_sum(dfn[u] + 1, dfn[v], 1, 1, n));
    return res;
  };
  // 将u-v路径上的边权加上val
  auto path_add = [&](int64_t u, int64_t v, int64_t val) {
    while (top[u] != top[v]) {
      if (depth[top[u]] < depth[top[v]]) { swap(u, v); }
      seg.range_add(dfn[top[u]], dfn[u], val, 1, 1, n);
      u = parent[top[u]];
    }
    if (depth[u] > depth[v]) { swap(u, v); }
    seg.range_add(dfn[u] + 1, dfn[v], val, 1, 1, n);
  };

  for (int i = 0; i < m; ++i) {
    char op;
    int u, v;
    cin >> op >> u >> v;
    if (op == 'P') {
      path_add(u, v, 1);
    } else {
      int64_t ans = path_sum(u, v);
      cout << ans << "\n";
    }
  }

  return 0;
}
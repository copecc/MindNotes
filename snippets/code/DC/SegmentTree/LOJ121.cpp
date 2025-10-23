#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>
using namespace std;

struct query {
  int64_t u, v, t, op;

  query() : u(0), v(0), t(0), op(0) {}

  query(int64_t _u, int64_t _v, int64_t _t, int64_t _op) : u(_u), v(_v), t(_t), op(_op) {}

  // 按边与时间排序
  bool operator<(const query &other) const {
    if (u != other.u) { return u < other.u; }
    if (v != other.v) { return v < other.v; }
    return t < other.t;
  }

  // 根据u和v判断是否为同一条边
  bool operator==(const query &other) const { return u == other.u && v == other.v; }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, m;
  cin >> n >> m;

  vector<query> queries(m + 1);
  for (int64_t i = 1; i <= m; ++i) {
    int64_t op, u, v;
    cin >> op >> u >> v;
    queries[i] = {min(u, v), max(u, v), i, op};
  }

  // 根据时间轴建立线段树
  vector<vector<pair<int64_t, int64_t>>> interval(4 * m);
  auto add = [&](auto &self, int64_t ql, int64_t qr, int64_t qx, int64_t qy, int64_t i, int64_t l,
                 int64_t r) -> void {
    if (ql <= l && r <= qr) {
      interval[i].emplace_back(qx, qy);
      return;
    }
    int64_t mid = (l + r) >> 1;
    if (ql <= mid) { self(self, ql, qr, qx, qy, 2 * i, l, mid); }
    if (qr > mid) { self(self, ql, qr, qx, qy, 2 * i + 1, mid + 1, r); }
  };
  // 处理所有边的添加和删除操作，构建线段树
  vector<int64_t> index(m + 1, 0);
  iota(index.begin(), index.end(), 0);
  sort(index.begin() + 1, index.end(),
       [&](int64_t a, int64_t b) { return queries[a] < queries[b]; });
  for (int64_t i = 1, j = 1; i <= m; i = j) {
    int64_t u = queries[index[i]].u, v = queries[index[i]].v;
    while (j <= m && queries[index[i]] == queries[index[j]]) { ++j; }  // 按边分组
    int64_t last_add = 0;
    for (int64_t k = i; k < j; ++k) {
      if (queries[index[k]].op == 0) {  // 添加边
        last_add = k;
      }
      if (queries[index[k]].op == 1) {  // 删除边
        int64_t start = queries[index[last_add]].t;
        int64_t end   = queries[index[k]].t - 1;
        add(add, start, end, u, v, 1, 1, m);
        last_add = 0;
      }
    }
    if (last_add != 0) {  // 最后一条添加的边没有被删除, 则一直持续到最后
      int64_t start = queries[index[last_add]].t;
      int64_t end   = m;
      add(add, start, end, u, v, 1, 1, m);
    }
  }
  // 并查集，支持回滚
  vector<int64_t> root(n + 1);
  vector<int64_t> size(n + 1, 1);
  vector<pair<int64_t, int64_t>> rollback;
  iota(root.begin(), root.end(), 0);
  auto find = [&](int64_t x) {
    while (root[x] != x) { x = root[x]; }
    return x;
  };
  auto union_set = [&](int64_t x, int64_t y) {
    int64_t fx = find(x);
    int64_t fy = find(y);
    if (fx == fy) { return 0; }
    if (size[fx] < size[fy]) { swap(fx, fy); }
    root[fy]  = fx;
    size[fx] += size[fy];
    rollback.emplace_back(fy, fx);
    return 1;
  };
  auto union_rollback = [&]() {
    if (rollback.empty()) { return; }
    auto [fy, fx] = rollback.back();
    rollback.pop_back();
    root[fy]  = fy;
    size[fx] -= size[fy];
  };

  // 深度优先遍历线段树，处理每个时间点的查询
  vector<bool> results(m + 1);  // 存储查询结果
  auto dfs = [&](auto &self, int64_t i, int64_t l, int64_t r) -> void {
    int64_t union_count = 0;
    // 添加当前区间的所有边
    for (auto &[u, v] : interval[i]) { union_count += union_set(u, v); }
    if (l == r) {
      if (queries[l].op == 2) {  // 处理查询操作
        int64_t u  = queries[l].u;
        int64_t v  = queries[l].v;
        results[l] = (find(u) == find(v));
      }
    } else {
      int64_t mid = (l + r) >> 1;
      self(self, 2 * i, l, mid);
      self(self, 2 * i + 1, mid + 1, r);
    }
    // 回滚到之前的状态
    for (int64_t k = 0; k < union_count; ++k) { union_rollback(); }
  };
  dfs(dfs, 1, 1, m);
  for (int64_t i = 1; i <= m; ++i) {
    if (queries[i].op == 2) { cout << (results[i] ? "Y" : "N") << "\n"; }
  }
  return 0;
}
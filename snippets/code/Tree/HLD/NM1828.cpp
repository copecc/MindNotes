#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>
using namespace std;

const int64_t MOD = 1e9 + 7;

struct segment_tree {
  vector<int64_t> lcm;  // 区间最小公倍数

  explicit segment_tree(int64_t n) : lcm(n * 4) {}

  void push_up(int64_t i) { lcm[i] = std::lcm(lcm[2 * i], lcm[2 * i + 1]); }

  // 构建线段树
  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums,
             const vector<int64_t> &rank, const vector<int64_t> &colors) {
    if (left == right) {  // 叶子节点，进行初始化, 将dfn映射回节点编号
      int64_t index = rank[left];
      lcm[i]        = colors[index] == 1 ? nums[index] : 1;
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums, rank, colors);
    build(2 * i + 1, mid + 1, right, nums, rank, colors);
    push_up(i);
  }

  // 单点修改: point_set(x, val, 1, 1, n) 将下标 x 的值修改为 val
  void point_set(int64_t index, int64_t val, int64_t i, int64_t left, int64_t right) {
    if (left == index && right == index) {  // 到叶子，直接修改数组中的值
      lcm[i] = val;
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    if (index <= mid) {
      point_set(index, val, 2 * i, left, mid);
    } else {
      point_set(index, val, 2 * i + 1, mid + 1, right);
    }
    push_up(i);
  }

  int64_t range_lcm(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return lcm[i]; }
    int64_t mid = l + ((r - l) / 2);
    int64_t res = 1;
    if (ql <= mid) { res = std::lcm(res, range_lcm(ql, qr, 2 * i, l, mid)); }
    if (qr > mid) { res = std::lcm(res, range_lcm(ql, qr, 2 * i + 1, mid + 1, r)); }
    return res;
  }
};

int main() {
  int64_t n, q;
  cin >> n >> q;

  vector<int64_t> nums(n + 1);
  for (int64_t i = 1; i <= n; ++i) { cin >> nums[i]; }

  vector<int64_t> colors(n + 1);  // 0表示白色, 1表示黑色
  for (int64_t i = 1; i <= n; ++i) {
    char color;
    cin >> color;
    colors[i] = (color == 'B') ? 1 : 0;
  }

  vector<vector<int64_t>> tree(n + 1);
  for (int64_t i = 1; i < n; ++i) {
    int64_t u;
    int64_t v;
    cin >> u >> v;
    tree[u].push_back(v);
    tree[v].push_back(u);
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

  dfs(dfs, 1, -1);  // 计算父节点, 深度, 子树大小, 重儿子

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
  decompose(decompose, 1, 1);  // 重链剖分, 根节点为1, 链顶为自己

  segment_tree seg(n);
  seg.build(1, 1, n, nums, rank, colors);

  auto query_path = [&](int64_t u, int64_t v) {
    int64_t res = 1;
    while (top[u] != top[v]) {
      if (depth[top[u]] < depth[top[v]]) { swap(u, v); }
      res = std::lcm(res, seg.range_lcm(dfn[top[u]], dfn[u], 1, 1, n));
      u   = parent[top[u]];
    }
    if (depth[u] > depth[v]) { swap(u, v); }
    res = std::lcm(res, seg.range_lcm(dfn[u], dfn[v], 1, 1, n));
    return res;
  };

  for (int64_t i = 0; i < q; ++i) {
    int64_t op;
    int64_t x;
    cin >> op >> x;
    if (op == 1) {  // 修改颜色, 相当于单点修改
      colors[x]   ^= 1;
      int64_t val  = colors[x] == 1 ? nums[x] : 1;
      seg.point_set(dfn[x], val, 1, 1, n);
    } else if (op == 2) {
      // 只对结果取模！！！
      cout << query_path(1, x) % MOD << '\n';
    }
  }

  return 0;
}
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> sum;      // 区间和
  vector<int64_t> tag_add;  // 区间加法懒标记

  int64_t p;  // 模数

  explicit segment_tree(int64_t n, int64_t p) : sum(n * 4), tag_add(n * 4), p(p) {}

  void push_up(int64_t i) { sum[i] = sum[2 * i] + sum[2 * i + 1]; }

  // 构建线段树
  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums,
             const vector<int64_t> &rank) {
    if (left == right) {  // 叶子节点，进行初始化, 将dfn映射回节点编号
      sum[i] = nums[rank[left]] % p;
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums, rank);
    build(2 * i + 1, mid + 1, right, nums, rank);
    push_up(i);
  }

  void lazy_add(int64_t i, int64_t val, int64_t count) {
    sum[i]     = (sum[i] + count * val) % p;
    tag_add[i] = (tag_add[i] + val) % p;
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
    if (ql <= mid) { res = (res + range_sum(ql, qr, 2 * i, l, mid)) % p; }
    if (qr > mid) { res = (res + range_sum(ql, qr, 2 * i + 1, mid + 1, r)) % p; }
    return res;
  }
};

int main() {
  int n, m, r, p;
  cin >> n >> m >> r >> p;
  vector<int64_t> nums(n + 1);
  for (int i = 1; i <= n; ++i) { cin >> nums[i]; }
  vector<vector<int64_t>> tree(n + 1);
  for (int i = 1; i < n; ++i) {
    int u, v;
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
  dfs(dfs, r, -1);  // 计算父节点, 深度, 子树大小, 重儿子

  // 链顶, dfs序, dfs反序(dfn为i的节点编号)
  vector<int64_t> top(n + 1), dfn(n + 1), rank(n + 1);
  int64_t timer  = 0;
  auto decompose = [&](auto &self, int64_t u, int64_t t) -> void {
    top[u]      = t;
    dfn[u]      = ++timer;
    rank[timer] = u;
    if (heavy_son[u] != -1) { self(self, heavy_son[u], t); }  // (1)!
    for (int64_t v : tree[u]) {  // 处理轻儿子, 轻儿子各自成链，链顶为自己
      if (v != parent[u] && v != heavy_son[u]) { self(self, v, v); }
    }
  };
  decompose(decompose, r, r);  // 重链剖分, 根节点为r, 链顶为自己

  segment_tree seg(n + 1, p);
  seg.build(1, 1, n, nums, rank);  // 根据dfn构建线段树, 注意rank将dfn映射回节点编号

  auto path_sum = [&](int64_t u, int64_t v) {
    int64_t res = 0;
    while (top[u] != top[v]) {  // (2)!
      if (depth[top[u]] < depth[top[v]]) { swap(u, v); }
      res = (res + seg.range_sum(dfn[top[u]], dfn[u], 1, 1, n)) % p;
      u   = parent[top[u]];
    }
    if (depth[u] > depth[v]) { swap(u, v); }  // (3)!
    res = (res + seg.range_sum(dfn[u], dfn[v], 1, 1, n)) % p;
    return res;
  };

  auto path_add = [&](int64_t u, int64_t v, int64_t val) {
    while (top[u] != top[v]) {
      if (depth[top[u]] < depth[top[v]]) { swap(u, v); }
      seg.range_add(dfn[top[u]], dfn[u], val, 1, 1, n);
      u = parent[top[u]];
    }
    if (depth[u] > depth[v]) { swap(u, v); }
    seg.range_add(dfn[u], dfn[v], val, 1, 1, n);
  };

  for (int i = 0; i < m; ++i) {
    int op;
    cin >> op;
    if (op == 1) {
      int x, y, z;
      cin >> x >> y >> z;
      path_add(x, y, z);
    } else if (op == 2) {
      int x, y;
      cin >> x >> y;
      cout << path_sum(x, y) << "\n";
    } else if (op == 3) {
      int x, z;
      cin >> x >> z;
      seg.range_add(dfn[x], dfn[x] + size[x] - 1, z, 1, 1, n);  // (4)!
    } else {
      int x;
      cin >> x;
      cout << seg.range_sum(dfn[x], dfn[x] + size[x] - 1, 1, 1, n) << "\n";
    }
  }
  return 0;
}
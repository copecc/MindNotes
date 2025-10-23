#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <set>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;

using PII  = pair<int, int>;
using TIII = tuple<int, int, int>;

struct segment_tree {
  vector<int> sum;          // 区间和
  vector<int> tag_set;      // 区间赋值懒标记
  vector<int> tag_set_val;  // 区间赋值懒标记值, 只有tag_set为true时该值才有意义

  explicit segment_tree(int n) : sum(n * 4), tag_set(n * 4), tag_set_val(n * 4) {}

  void push_up(int i) { sum[i] = sum[2 * i] + sum[2 * i + 1]; }

  // 构建线段树
  void build(int i, int left, int right) {
    if (left == right) {  // 叶子节点，进行初始化, 将dfn映射回节点编号
      sum[i] = 1;
      return;
    }
    int mid = left + ((right - left) / 2);
    build(2 * i, left, mid);
    build(2 * i + 1, mid + 1, right);
    push_up(i);
  }

  void lazy_set(int i, int val, int count) {
    sum[i]         = count * val;
    tag_set[i]     = 1;
    tag_set_val[i] = val;
  }

  // 向下传递懒标记
  void push_down(int i, int left_count, int right_count) {
    if (tag_set[i] != 0) {  // 处理赋值
      lazy_set(2 * i, tag_set_val[i], left_count);
      lazy_set(2 * i + 1, tag_set_val[i], right_count);
      tag_set[i] = 0;  // 清空根节点赋值标记
    }
  }

  // 区间赋值: range_set(x, y, val, 1, 1, n) 将区间 [x,y] 的值修改为 val
  void range_set(int ql, int qr, int val, int i, int l, int r) {
    if (ql <= l && r <= qr) {  // 区间覆盖, 直接更新
      lazy_set(i, val, r - l + 1);
      return;
    }
    int mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    if (ql <= mid) { range_set(ql, qr, val, 2 * i, l, mid); }
    if (qr > mid) { range_set(ql, qr, val, 2 * i + 1, mid + 1, r); }
    push_up(i);
  }

  // 区间求和: range_sum(x, y, 1, 1, n) 查询区间 [x,y] 的和
  int range_sum(int ql, int qr, int i, int l, int r) {
    if (ql <= l && r <= qr) { return sum[i]; }  // 区间覆盖，直接返回
    int mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    // 汇总结果
    int res = 0;
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

  set<PII> edges;  // 存储当前图中的边, 每条边只存一次, (u,v): u<v
  for (int i = 0; i < m; ++i) {
    int u, v;
    cin >> u >> v;
    edges.insert({min(u, v), max(u, v)});
  }

  vector<TIII> operations;
  int op;
  while (cin >> op) {
    if (op == -1) { break; }
    int u, v;
    cin >> u >> v;
    if (op == 0) {  // 删除操作
      operations.emplace_back(0, min(u, v), max(u, v));
      edges.erase({min(u, v), max(u, v)});  // 不能在生成树中使用该边
    } else {
      operations.emplace_back(1, min(u, v), max(u, v));  // 查询
    }
  }

  // 并查集, 构造生成树
  vector<int> root(n + 1);
  iota(root.begin(), root.end(), 0);
  function<int(int)> find;
  find = [&](int x) { return root[x] == x ? x : root[x] = find(root[x]); };

  vector<vector<int>> tree(n + 1);
  for (const auto &[u, v] : edges) {
    int ru = find(u);
    int rv = find(v);
    if (ru != rv) {  // 树边
      root[rv] = ru;
      tree[u].push_back(v);
      tree[v].push_back(u);
    } else {                             // 剩下的非树边
      operations.emplace_back(0, u, v);  // 相当于删除操作, 最后需要优先删除操作的边进行操作
    }
  }
  // 重链剖分准备
  int r = 1;  // 以1号节点为根节点
  // 父节点, 深度, 子树大小, 重儿子
  vector<int> parent(n + 1), depth(n + 1), size(n + 1), heavy_son(n + 1, -1);
  auto dfs = [&](auto &self, int u, int from) -> void {
    parent[u]    = from;
    size[u]      = 1;
    int max_size = 0;
    for (int v : tree[u]) {
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
  vector<int> top(n + 1), dfn(n + 1), rank(n + 1);
  int timer      = 0;
  auto decompose = [&](auto &self, int u, int t) -> void {
    top[u]      = t;
    dfn[u]      = ++timer;
    rank[timer] = u;
    if (heavy_son[u] != -1) { self(self, heavy_son[u], t); }
    for (int v : tree[u]) {  // 处理轻儿子, 轻儿子各自成链，链顶为自己
      if (v != parent[u] && v != heavy_son[u]) { self(self, v, v); }
    }
  };
  decompose(decompose, r, r);  // 重链剖分, 根节点为r, 链顶为自己
  // 构建线段树, 一开始都是关键边, 所以点权都为1
  segment_tree seg(n);
  seg.build(1, 1, n);
  // 边权下放到子节点的点权上, 根节点清0
  seg.range_set(1, 1, 0, 1, 1, n);
  // 查询u-v路径上的关键边数
  auto path_sum = [&](int u, int v) {
    int res = 0;
    while (top[u] != top[v]) {
      if (depth[top[u]] < depth[top[v]]) { swap(u, v); }
      res = (res + seg.range_sum(dfn[top[u]], dfn[u], 1, 1, n));
      u   = parent[top[u]];
    }
    if (depth[u] > depth[v]) { swap(u, v); }
    res = (res + seg.range_sum(dfn[u] + 1, dfn[v], 1, 1, n));
    return res;
  };
  // 路径赋值, 此处用于将u-v之间的边权清0
  auto path_set = [&](int u, int v, int val) {
    while (top[u] != top[v]) {
      if (depth[top[u]] < depth[top[v]]) { swap(u, v); }
      seg.range_set(dfn[top[u]], dfn[u], val, 1, 1, n);
      u = parent[top[u]];
    }
    if (depth[u] > depth[v]) { swap(u, v); }
    seg.range_set(dfn[u] + 1, dfn[v], val, 1, 1, n);
  };

  vector<int> answer;
  for (int i = operations.size() - 1; i >= 0; --i) {
    auto [op, u, v] = operations[i];
    if (op == 0) {  // 删除操作, 逆着就是将边加回去, uv之间的树边都变成非关键边
      path_set(u, v, 0);
    } else {
      int ans = path_sum(u, v);
      answer.push_back(ans);
    }
  }
  reverse(answer.begin(), answer.end());
  for (int ans : answer) { cout << ans << "\n"; }

  return 0;
}
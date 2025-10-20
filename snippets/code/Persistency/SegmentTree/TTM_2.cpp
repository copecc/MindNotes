#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct node {
  int64_t left;
  int64_t right;
  int64_t sum;      // 区间和
  int64_t tag_add;  // 懒惰标记
};

vector<node> nodes;  // 所有节点

struct tag_p_segment_tree {
  tag_p_segment_tree() {
    nodes.clear();               // 清空节点
    nodes.emplace_back(node{});  // 占位, 节点编号从1开始
  }

  // 构建0版本线段树, 返回根节点编号
  int64_t build(int64_t left, int64_t right, const vector<int64_t> &nums) {
    int64_t root = nodes.size();  // 新节点编号
    nodes.emplace_back(node{});   // 创建新节点
    if (left == right) {          // 叶子节点
      nodes[root].sum = nums[left];
      return root;
    }
    int64_t mid       = left + ((right - left) / 2);
    nodes[root].left  = build(left, mid, nums);
    nodes[root].right = build(mid + 1, right, nums);
    nodes[root].sum   = nodes[nodes[root].left].sum + nodes[nodes[root].right].sum;
    return root;
  }

  static int64_t clone(int64_t i) {
    int64_t new_root = nodes.size();  // 新节点编号
    nodes.push_back(nodes[i]);        // 克隆当前节点
    return new_root;
  }

  int64_t range_add(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    int64_t new_node     = clone(i);
    nodes[new_node].sum += val * (min(qr, r) - max(ql, l) + 1);
    if (ql <= l && r <= qr) {  // 区间覆盖
      nodes[new_node].tag_add += val;
      return new_node;
    }
    int64_t mid = l + ((r - l) / 2);
    if (ql <= mid) { nodes[new_node].left = range_add(ql, qr, val, nodes[new_node].left, l, mid); }
    if (qr > mid) {
      nodes[new_node].right = range_add(ql, qr, val, nodes[new_node].right, mid + 1, r);
    }
    return new_node;
  }

  // 查询区间和, total_add为从根节点到当前节点路径上所有懒惰标记的和
  int64_t range_sum(int64_t ql, int64_t qr, int64_t added, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return nodes[i].sum + added * (r - l + 1); }
    int64_t mid = l + ((r - l) / 2);
    // 汇总结果
    int64_t res = 0;
    if (ql <= mid) {
      res += range_sum(ql, qr, added + nodes[i].tag_add, nodes[i].left, l, mid);
    }
    if (qr > mid) {
      res += range_sum(ql, qr, added + nodes[i].tag_add, nodes[i].right, mid + 1, r);
    }
    return res;
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int64_t n, m;
  cin >> n >> m;

  nodes.reserve(70 * n + 20 * m);  // 经验值
  vector<int64_t> nums(n + 1);
  for (int64_t i = 1; i <= n; i++) { cin >> nums[i]; }
  tag_p_segment_tree tree;

  vector<int64_t> roots(m + 1);  // 各个版本的根节点
  roots[0]  = tree.build(1, n, nums);
  int64_t t = 0;
  for (int64_t i = 0; i < m; i++) {
    string op;
    cin >> op;
    if (op == "C") {
      int64_t l, r, val;
      cin >> l >> r >> val;
      int64_t new_root = tree.range_add(l, r, val, roots[t], 1, n);
      roots[++t]       = new_root;
    } else if (op == "Q") {
      int64_t l, r;
      cin >> l >> r;
      cout << tree.range_sum(l, r, 0, roots[t], 1, n) << "\n";
    } else if (op == "H") {
      int64_t l, r, version;
      cin >> l >> r >> version;
      cout << tree.range_sum(l, r, 0, roots[version], 1, n) << "\n";
    } else {  // op == "B"
      int64_t version;
      cin >> version;
      t = version;
    }
  }
  return 0;
}
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

struct p_segment_tree {
  p_segment_tree() {
    nodes.clear();               // 清空节点
    nodes.emplace_back(node{});  // 占位, 节点编号从1开始
  }

  static void push_up(int64_t root) {
    nodes[root].sum = nodes[nodes[root].left].sum + nodes[nodes[root].right].sum;
  }

  // 构建0版本线段树, 返回根节点编号
  int64_t build(int64_t left, int64_t right, const vector<int64_t> &nums) {
    int64_t i = nodes.size();  // 新节点编号
    nodes.emplace_back(node{});   // 创建新节点
    if (left == right) {          // 叶子节点
      nodes[i].sum = nums[left];
      return i;
    }
    int64_t mid       = left + ((right - left) / 2);
    nodes[i].left  = build(left, mid, nums);
    nodes[i].right = build(mid + 1, right, nums);
    // 更新当前节点的区间和
    push_up(i);
    return i;
  }

  static void lazy_add(int64_t i, int64_t val, int64_t count) {
    nodes[i].sum     += count * val;
    nodes[i].tag_add += val;
  }

  static int64_t clone(int64_t i) {
    int64_t new_node = nodes.size();  // 新节点编号
    nodes.emplace_back(nodes[i]);     // 克隆当前节点
    return new_node;
  }

  // 向下传递懒惰标记
  static void push_down(int64_t i, int64_t left_count, int64_t right_count) {
    if (nodes[i].tag_add != 0) {               // 处理加法
      nodes[i].left  = clone(nodes[i].left);   // 克隆左子节点
      nodes[i].right = clone(nodes[i].right);  // 克隆右子节点
      // 子节点各自加上更新值
      lazy_add(nodes[i].left, nodes[i].tag_add, left_count);
      lazy_add(nodes[i].right, nodes[i].tag_add, right_count);
      nodes[i].tag_add = 0;  // 清空根节点加法标记
    }
  }

  int64_t range_add(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    int64_t new_node = clone(i);
    if (ql <= l && r <= qr) {  // 区间覆盖
      lazy_add(new_node, val, r - l + 1);
      return new_node;
    }
    int64_t mid = l + ((r - l) / 2);
    push_down(new_node, mid - l + 1, r - mid);
    if (ql <= mid) { nodes[new_node].left = range_add(ql, qr, val, nodes[new_node].left, l, mid); }
    if (qr > mid) {
      nodes[new_node].right = range_add(ql, qr, val, nodes[new_node].right, mid + 1, r);
    }
    push_up(new_node);
    return new_node;
  }

  // 查询区间和
  int64_t range_sum(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return nodes[i].sum; }
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    // 汇总结果
    int64_t res = 0;
    if (ql <= mid) { res += range_sum(ql, qr, nodes[i].left, l, mid); }
    if (qr > mid) { res += range_sum(ql, qr, nodes[i].right, mid + 1, r); }
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
  p_segment_tree tree;

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
      cout << tree.range_sum(l, r, roots[t], 1, n) << "\n";
    } else if (op == "H") {
      int64_t l, r, version;
      cin >> l >> r >> version;
      cout << tree.range_sum(l, r, roots[version], 1, n) << "\n";
    } else {  // op == "B"
      int64_t version;
      cin >> version;
      t = version;
    }
  }
  return 0;
}
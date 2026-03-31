#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct node {
  int64_t left;
  int64_t right;
  int64_t size;  // 记录该节点区间内有多少个元素
};

vector<node> nodes;

struct p_segment_tree {
  explicit p_segment_tree() {
    nodes.clear();               // 清空节点
    nodes.emplace_back(node{});  // 占位, 节点编号从1开始
  }

  // 更新当前节点的出现次数
  void push_up(int64_t i) {
    nodes[i].size = nodes[nodes[i].left].size + nodes[nodes[i].right].size;
  }

  // 构建0号版本的线段树, 没有任何元素
  int64_t build(int64_t left, int64_t right) {
    int64_t i = nodes.size();  // 新节点编号
    nodes.emplace_back(node{});
    if (left == right) { return i; }
    int64_t mid    = left + ((right - left) / 2);
    nodes[i].left  = build(left, mid);
    nodes[i].right = build(mid + 1, right);
    push_up(i);
    return i;
  }

  static int64_t clone(int64_t i) {
    int64_t new_node = nodes.size();  // 新节点编号
    nodes.emplace_back(nodes[i]);     // 克隆当前节点
    return new_node;
  }

  // 记录值出现一次, 返回新版本的根节点
  int64_t update(int64_t index, int64_t i, int64_t l, int64_t r) {
    int64_t new_node = clone(i);  // 克隆当前节点
    if (l == r) {                 // 叶子节点
      nodes[new_node].size += 1;  // 该节点出现次数加1
      return new_node;
    }
    int64_t mid = l + ((r - l) / 2);
    if (index <= mid) {
      nodes[new_node].left = update(index, nodes[i].left, l, mid);
    } else {
      nodes[new_node].right = update(index, nodes[i].right, mid + 1, r);
    }
    push_up(new_node);
    return new_node;
  }

  // 查询出现次数大于k的元素
  void query(int64_t k, int64_t root_u, int64_t root_v, int64_t l, int64_t r,
             vector<int64_t> &ans) {
    if (nodes[root_v].size - nodes[root_u].size <= k) { return; }  // 区间内元素总数不超过k
    if (l == r) {                                                  // 叶子节点, 出现次数大于k
      ans.push_back(l);
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    query(k, nodes[root_u].left, nodes[root_v].left, l, mid, ans);
    query(k, nodes[root_u].right, nodes[root_v].right, mid + 1, r, ans);
  }
};

int solve() {
  int64_t n, q;
  cin >> n >> q;
  nodes.reserve(n * 40);  // 预分配空间
  vector<int64_t> nums(n + 1);
  for (int64_t i = 1; i <= n; i++) { cin >> nums[i]; }
  vector<int64_t> sorted_nums = nums;
  sort(sorted_nums.begin() + 1, sorted_nums.end());

  auto get_rank = [&](int64_t v) {  // 获取值的排名, 从1开始
    return lower_bound(sorted_nums.begin() + 1, sorted_nums.end(), v) - sorted_nums.begin();
  };

  vector<int64_t> roots(n + 1);
  p_segment_tree seg_tree;
  roots[0] = seg_tree.build(1, n);
  for (int64_t i = 1; i <= n; i++) {  // 构建每个版本的线段树, 记录前i个元素
    int64_t rank = get_rank(nums[i]);
    roots[i]     = seg_tree.update(rank, roots[i - 1], 1, n);
  }
  for (int64_t i = 0; i < q; i++) {
    int64_t l, r;
    cin >> l >> r;
    int64_t k = (r - l + 1) / 3;
    vector<int64_t> ans;
    seg_tree.query(k, roots[l - 1], roots[r], 1, n, ans);
    for (int64_t x : ans) { cout << sorted_nums[x] << " "; }
    if (ans.empty()) { cout << "-1"; }
    cout << "\n";
  }

  return 0;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t t = 1;
  cin >> t;
  while ((t--) != 0) { solve(); }
  return 0;
}
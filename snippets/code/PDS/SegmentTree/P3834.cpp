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

vector<node> nodes;  // 所有节点

struct p_segment_tree {
  p_segment_tree() {
    nodes.clear();               // 清空节点
    nodes.emplace_back(node{});  // 占位, 节点编号从1开始
  }

  // 更新当前节点的出现次数
  void push_up(int64_t i) {
    nodes[i].size = nodes[nodes[i].left].size + nodes[nodes[i].right].size;
  }

  // 构建线段树, 返回根节点编号
  int64_t build(int64_t left, int64_t right) {
    int64_t i = nodes.size();    // 新节点编号
    nodes.emplace_back(node{});  // 创建新节点
    if (left == right) {         // 叶子节点
      nodes[i].size = 0;         // 初始化为0
      return i;
    }
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

  // 修改节点的值, 返回新版本的根节点
  int64_t update(int64_t index, int64_t i, int64_t l, int64_t r) {
    int64_t new_node = clone(i);  // 克隆当前节点
    if (l == r) {
      nodes[new_node].size += 1;
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

  // 查询第k小元素, root_u为版本u的根节点, root_v为版本v的根节点
  int64_t query(int64_t k, int64_t root_u, int64_t root_v, int64_t l, int64_t r) {
    if (l == r) { return l; }
    int64_t mid = l + ((r - l) / 2);
    // 计算左子树中元素的数量
    int64_t left_count = nodes[nodes[root_v].left].size - nodes[nodes[root_u].left].size;
    // 如果k小于等于左子树中的元素数量, 则第k小元素在左子树中, 否则在右子树中
    if (k <= left_count) { return query(k, nodes[root_u].left, nodes[root_v].left, l, mid); }
    return query(k - left_count, nodes[root_u].right, nodes[root_v].right, mid + 1, r);
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
  vector<int64_t> sorted_nums = nums;
  sort(sorted_nums.begin() + 1, sorted_nums.end());

  vector<int64_t> roots(n + 1);
  p_segment_tree seg;
  roots[0]      = seg.build(1, n);  // 构建初始版本0

  auto get_rank = [&](int64_t x) {  // 离散化, 映射到1~n
    return lower_bound(sorted_nums.begin() + 1, sorted_nums.end(), x) - sorted_nums.begin();
  };
  for (int64_t i = 1; i <= n; i++) {
    int64_t index = get_rank(nums[i]);  // 获得nums[i]的排名
    roots[i]      = seg.update(index, roots[i - 1], 1, n);
  }
  for (int64_t i = 0; i < m; i++) {
    int64_t l, r, k;
    cin >> l >> r >> k;
    int64_t index = seg.query(k, roots[l - 1], roots[r], 1, n);
    cout << sorted_nums[index] << "\n";  // 映射回原值
  }
  return 0;
}
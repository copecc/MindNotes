#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct node {
  int64_t left;   // 左子节点编号
  int64_t right;  // 右子节点编号
  int64_t value;  // 叶子节点存储值, 非叶子节点可不存储
};

vector<node> nodes;  // 所有节点

struct p_array {
  explicit p_array() {
    nodes.clear();               // 清空节点
    nodes.emplace_back(node{});  // 占位, 节点编号从1开始
  }

  int64_t build(int64_t left, int64_t right, const vector<int64_t> &nums) {
    int64_t i = nodes.size();  // 新节点编号
    nodes.emplace_back(node{});
    if (left == right) {  // 叶子节点
      nodes[i].value = nums[left];
      return i;
    }
    int64_t mid    = left + ((right - left) / 2);
    nodes[i].left  = build(left, mid, nums);
    nodes[i].right = build(mid + 1, right, nums);
    return i;
  }

  static int64_t clone(int64_t i) {
    int64_t new_node = nodes.size();  // 新节点编号
    nodes.emplace_back(nodes[i]);     // 克隆当前节点
    return new_node;
  }

  // 修改节点的值, 返回新版本的根节点
  // update(x, val, root, 1, n) 表示将版本v中下标x的值修改为val, root为版本v的根节点
  int64_t update(int64_t index, int64_t val, int64_t i, int64_t l, int64_t r) {
    int64_t new_node = clone(i);  // 克隆当前节点
    if (l == r) {                 // 叶子节点
      nodes[new_node].value = val;
      return new_node;
    }
    int64_t mid = l + ((r - l) / 2);
    if (index <= mid) {
      nodes[new_node].left = update(index, val, nodes[i].left, l, mid);
    } else {
      nodes[new_node].right = update(index, val, nodes[i].right, mid + 1, r);
    }
    return new_node;
  }

  // 查询某个版本的某个下标的值
  // query(x, root, 1, n) 表示查询版本v中下标x的值, root为版本v的根节点
  int64_t query(int64_t index, int64_t i, int64_t l, int64_t r) {
    if (l == r) { return nodes[i].value; }
    int64_t mid = l + ((r - l) / 2);
    if (index <= mid) { return query(index, nodes[i].left, l, mid); }
    return query(index, nodes[i].right, mid + 1, r);
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
  p_array tree;

  vector<int64_t> root(m + 1);
  root[0] = tree.build(1, n, nums);  // 构建初始版本0

  int64_t v, op;
  for (int64_t i = 1; i <= m; i++) {
    cin >> v >> op;
    if (op == 1) {  // 修改
      int64_t p, c;
      cin >> p >> c;
      root[i] = tree.update(p, c, root[v], 1, n);
    } else if (op == 2) {  // 查询
      int64_t p;
      cin >> p;
      cout << tree.query(p, root[v], 1, n) << "\n";
      root[i] = root[v];  // 新版本和旧版本相同
    }
  }
  return 0;
}
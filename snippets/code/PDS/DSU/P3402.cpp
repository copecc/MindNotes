#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct node {
  int64_t left;   // 左子节点编号
  int64_t right;  // 右子节点编号
  int64_t root;   // 父节点编号
  int64_t size;   // 集合大小
};

vector<node> nodes;  // 所有节点

struct p_dsu {
  explicit p_dsu() {
    nodes.clear();               // 清空节点
    nodes.emplace_back(node{});  // 占位, 节点编号从1开始
  }

  int64_t build(int64_t left, int64_t right) {
    int64_t i = nodes.size();  // 新节点编号
    nodes.emplace_back(node{});
    if (left == right) {
      nodes[i].root = left;  // 初始时每个节点的父节点是自己
      nodes[i].size = 1;     // 初始时每个集合大小为1
      return i;
    }
    int64_t mid    = left + ((right - left) / 2);
    nodes[i].left  = build(left, mid);
    nodes[i].right = build(mid + 1, right);
    return i;
  }

  static int64_t clone(int64_t i) {
    int64_t new_node = nodes.size();  // 新节点编号
    nodes.emplace_back(nodes[i]);     // 克隆当前节点
    return new_node;
  }

  // 更新某个版本的某个下标的父节点或集合大小
  int64_t update(int64_t q_i, int64_t q_v, bool q_root, int64_t i, int64_t l, int64_t r) {
    int64_t new_node = clone(i);  // 克隆当前节点
    if (l == r) {                 // 叶子节点
      if (q_root) {
        nodes[new_node].root = q_v;  // 更新父节点
      } else {
        nodes[new_node].size = q_v;  // 更新集合大小
      }
      return new_node;
    }
    int64_t mid = l + ((r - l) / 2);
    if (q_i <= mid) {
      nodes[new_node].left = update(q_i, q_v, q_root, nodes[i].left, l, mid);
    } else {
      nodes[new_node].right = update(q_i, q_v, q_root, nodes[i].right, mid + 1, r);
    }
    return new_node;
  }

  // 查询某个版本的某个下标的父节点或集合大小
  int64_t query(int64_t q_i, bool q_root, int64_t i, int64_t l, int64_t r) {
    if (l == r) { return q_root ? nodes[i].root : nodes[i].size; }
    int64_t mid = l + ((r - l) / 2);
    if (q_i <= mid) { return query(q_i, q_root, nodes[i].left, l, mid); }
    return query(q_i, q_root, nodes[i].right, mid + 1, r);
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int64_t n, m;
  cin >> n >> m;

  nodes.reserve(40 * n);  // 预分配节点空间
  p_dsu dsu;

  auto find = [&](int64_t x, int64_t root) {
    int64_t fx = dsu.query(x, true, root, 1, n);
    while (fx != x) {
      x  = fx;
      fx = dsu.query(x, true, root, 1, n);
    }
    return fx;
  };

  auto union_set = [&](int64_t x, int64_t y, int64_t root_v) {
    int64_t fx = find(x, root_v);
    int64_t fy = find(y, root_v);
    if (fx == fy) { return root_v; };  // 已经在同一集合中
    int64_t root_u = 0;                // 新版本根节点 (1)
    int64_t size_x = dsu.query(fx, false, root_v, 1, n);
    int64_t size_y = dsu.query(fy, false, root_v, 1, n);
    if (size_x < size_y) {
      swap(fx, fy);
      swap(size_x, size_y);
    }
    // 将fy的父节点设为fx
    root_u = dsu.update(fy, fx, true, root_v, 1, n);
    // 更新fx的集合大小
    root_u = dsu.update(fx, size_x + size_y, false, root_u, 1, n);
    return root_u;
  };

  vector<int64_t> root(m + 1);  // 版本记录
  root[0] = dsu.build(1, n);    // 初始版本

  for (int64_t v = 1; v <= m; ++v) {
    int64_t op;
    cin >> op;
    if (op == 1) {  // 合并操作
      int64_t x, y;
      cin >> x >> y;
      root[v] = union_set(x, y, root[v - 1]);  // 合并操作, 更新当前版本
    } else if (op == 2) {
      int64_t x;
      cin >> x;
      root[v] = root[x];  // 回滚到某个版本
    } else {
      int64_t x, y;
      cin >> x >> y;
      int64_t fx = find(x, root[v - 1]);
      int64_t fy = find(y, root[v - 1]);
      if (fx == fy) {
        cout << "1\n";
      } else {
        cout << "0\n";
      }
      root[v] = root[v - 1];  // 默认继承上一个版本
    }
  }

  return 0;
}
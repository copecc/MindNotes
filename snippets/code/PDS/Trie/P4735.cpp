#include <array>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct node {
  array<int64_t, 2> children;
  int64_t pass;
};

vector<node> nodes;  // 所有节点

struct p_trie {
  const int64_t BIT = 32;  // 假设数字不超过32位

  p_trie() {
    nodes.clear();               // 清空节点
    nodes.emplace_back(node{});  // 占位, 节点编号从1开始
  }

  static int64_t clone(int64_t i) {
    int64_t new_node = nodes.size();  // 新节点编号
    nodes.emplace_back(nodes[i]);     // 克隆当前节点
    return new_node;
  }

  // 插入一个数字, 返回新版本的根节点
  int64_t insert(int64_t num, int64_t i) const {
    int64_t new_node      = clone(i);  // 克隆当前节点
    nodes[new_node].pass += 1;
    for (int64_t bit = BIT - 1, last = new_node; bit >= 0; --bit) {
      int64_t current_bit                = (num >> bit) & 1;
      i                                  = nodes[last].children[current_bit];
      int64_t new_child                  = clone(i);
      nodes[new_child].pass             += 1;
      nodes[last].children[current_bit]  = new_child;
      last                               = new_child;
    }
    return new_node;
  }

  // root_u为版本u的根节点, root_v为版本v的根节点
  int64_t query(int64_t num, int64_t root_u, int64_t root_v) const {
    int64_t result = 0;
    for (int64_t bit = BIT - 1; bit >= 0; --bit) {
      int64_t current_bit = (num >> bit) & 1;
      int64_t desired_bit = current_bit ^ 1;  // 希望走这条路
      int64_t count_in_v  = nodes[nodes[root_v].children[desired_bit]].pass;
      int64_t count_in_u  = nodes[nodes[root_u].children[desired_bit]].pass;
      if (count_in_v - count_in_u > 0) {  // 说明在u-v区间内存在这条路
        result += (1LL << bit);
        root_u  = nodes[root_u].children[desired_bit];
        root_v  = nodes[root_v].children[desired_bit];
      } else {  // 只能走另一条路
        root_u = nodes[root_u].children[current_bit];
        root_v = nodes[root_v].children[current_bit];
      }
    }
    return result;
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, m;
  cin >> n >> m;
  p_trie trie;
  vector<int64_t> roots(n + 1);

  int64_t eor = 0;
  // 空版本的根节点编号为0
  // 构建0版本
  roots[0] = trie.insert(eor, 0);
  for (int64_t i = 1; i <= n; ++i) {
    int64_t num;
    cin >> num;
    eor      ^= num;
    roots[i]  = trie.insert(eor, roots[i - 1]);
  }

  for (int64_t i = 0; i < m; ++i) {
    char op;
    cin >> op;
    if (op == 'A') {
      int64_t num;
      cin >> num;
      eor ^= num;
      roots.push_back(trie.insert(eor, roots.back()));
    } else if (op == 'Q') {
      int64_t l, r, x;
      cin >> l >> r >> x;
      cout << (trie.query(eor ^ x, l == 1 ? 0 : roots[l - 2], roots[r - 1])) << "\n";
    }
  }
  return 0;
}
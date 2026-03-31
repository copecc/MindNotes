#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct node {
  int64_t left;
  int64_t right;
  int64_t last_pos;
};

vector<node> nodes;  // 所有节点

struct p_segment_tree {
  p_segment_tree() {
    nodes.clear();               // 清空节点
    nodes.emplace_back(node{});  // 占位, 节点编号从1开始
  }

  // 更新当前区间元素的最小最后出现位置
  static void push_up(int64_t i) {
    int64_t left_min_pos  = nodes[nodes[i].left].last_pos;
    int64_t right_min_pos = nodes[nodes[i].right].last_pos;
    nodes[i].last_pos     = std::min(left_min_pos, right_min_pos);
  }

  static int64_t clone(int64_t i) {
    int64_t new_node = nodes.size();  // 新节点编号
    nodes.emplace_back(nodes[i]);     // 克隆当前节点
    return new_node;
  }

  // 修改节点的值, 返回新版本的根节点
  int64_t update(int64_t index, int64_t pos, int64_t i, int64_t l, int64_t r) {
    int64_t new_node = clone(i);  // 克隆当前节点
    if (l == r) {
      nodes[new_node].last_pos = pos;  // 更新最后出现位置
      return new_node;
    }
    int64_t mid = l + ((r - l) / 2);
    if (index <= mid) {
      nodes[new_node].left = update(index, pos, nodes[i].left, l, mid);
    } else {
      nodes[new_node].right = update(index, pos, nodes[i].right, mid + 1, r);
    }
    push_up(new_node);
    return new_node;
  }

  // root_u为版本u的根节点
  int64_t query(int64_t pos, int64_t root_u, int64_t l, int64_t r) {
    if (l == r) { return l; }
    int64_t mid          = l + ((r - l) / 2);
    int64_t left_min_pos = nodes[nodes[root_u].left].last_pos;
    // 如果左侧的最小最后出现位置小于pos, 说明在左子树中存在小于pos的值
    if (left_min_pos < pos) { return query(pos, nodes[root_u].left, l, mid); }
    // 否则左子树的值都在 pos 以后出现了, 去右子树找
    return query(pos, nodes[root_u].right, mid + 1, r);
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int64_t n, m;
  cin >> n >> m;
  vector<int64_t> nums(n + 1);
  for (int64_t i = 1; i <= n; ++i) { cin >> nums[i]; }
  p_segment_tree pst;
  vector<int64_t> roots(n + 1);
  for (int64_t i = 1; i <= n; ++i) {
    if (nums[i] < 0 || nums[i] > n) {  // 数字越界, 不更新
      roots[i] = roots[i - 1];
      continue;
    }
    roots[i] = pst.update(nums[i], i, roots[i - 1], 0, n);
  }
  for (int64_t i = 0; i < m; ++i) {
    int64_t l, r;
    cin >> l >> r;
    int64_t ans = pst.query(l, roots[r], 0, n);
    cout << ans << "\n";
  }
  return 0;
}
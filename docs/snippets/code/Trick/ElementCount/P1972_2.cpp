#include <algorithm>
#include <iostream>
#include <tuple>
#include <vector>
using namespace std;

struct node {
  int left;
  int right;
  int size;  // 记录该节点区间内有多少个元素
};

vector<node> nodes;  // 所有节点

struct p_segment_tree {
  p_segment_tree() {
    nodes.clear();               // 清空节点
    nodes.emplace_back(node{});  // 占位, 节点编号从1开始
  }

  // 更新当前节点的出现次数
  static void push_up(int i) {
    nodes[i].size = nodes[nodes[i].left].size + nodes[nodes[i].right].size;
  }

  static int clone(int i) {
    int new_node = nodes.size();   // 新节点编号
    nodes.emplace_back(nodes[i]);  // 克隆当前节点
    return new_node;
  }

  // 修改节点的值, 返回新版本的根节点
  int update(int index, int val, int i, int l, int r) {
    int new_node = clone(i);  // 克隆当前节点
    if (l == r) {
      nodes[new_node].size += val;
      return new_node;
    }
    int mid = l + ((r - l) / 2);
    if (index <= mid) {
      nodes[new_node].left = update(index, val, nodes[i].left, l, mid);
    } else {
      nodes[new_node].right = update(index, val, nodes[i].right, mid + 1, r);
    }
    push_up(new_node);
    return new_node;
  }

  // 查询到左端点ql为止的区间元素个数
  int query(int ql, int root_v, int l, int r) {
    if (l == r) { return nodes[root_v].size; }
    int mid = l + ((r - l) / 2);
    if (ql <= mid) {
      return query(ql, nodes[root_v].left, l, mid) + nodes[nodes[root_v].right].size;
    }
    return query(ql, nodes[root_v].right, mid + 1, r);
  };
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n;
  cin >> n;
  nodes.reserve(40 * n);  // 预估节点数量，避免多次扩容
  vector<int> nums(n + 1);
  for (int i = 1; i <= n; i++) { cin >> nums[i]; }

  vector<int> sorted_nums = nums;
  sort(sorted_nums.begin() + 1, sorted_nums.end());
  auto get_rank = [&](int x) {  // 返回x的排名，排名从1开始
    return lower_bound(sorted_nums.begin() + 1, sorted_nums.end(), x) - sorted_nums.begin();
  };

  p_segment_tree pst;
  vector<int> roots(n + 1);    // roots[i]表示前i个数对应的线段树根节点
  int v = 0;                   // 当前处理到第v个数, roots[0]表示空树
  vector<int> last(n + 1, 0);  // last[i]表示第i个数上次出现的位置

  int m;
  cin >> m;

  using TIII = tuple<int, int, int>;  // left, right, id
  vector<TIII> queries(m);
  for (int i = 0; i < m; i++) {
    int left, right;
    cin >> left >> right;
    queries[i] = make_tuple(left, right, i);
  }
  sort(queries.begin(), queries.end(),
       [](const TIII &a, const TIII &b) { return get<1>(a) < get<1>(b); });

  vector<int> answers(m);
  for (auto [left, right, id] : queries) {
    while (v < right) {
      ++v;
      int rank         = get_rank(nums[v]);
      int current_root = roots[v - 1];
      // 取消上次出现
      if (last[rank] != 0) { current_root = pst.update(last[rank], -1, current_root, 1, n); }
      current_root = pst.update(v, 1, current_root, 1, n);  // 添加本次出现
      roots[v]     = current_root;
      last[rank]   = v;
    }
    answers[id] = pst.query(left, roots[right], 1, n);
  }

  for (const auto &ans : answers) { cout << ans << '\n'; }

  return 0;
}
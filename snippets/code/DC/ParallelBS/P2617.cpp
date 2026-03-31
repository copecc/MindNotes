#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct BIT {
  explicit BIT(int64_t n) : n(n), tree(n + 1) {}

  void point_add(int64_t x, int64_t delta) {
    for (; x <= n; x += lowbit(x)) { tree[x] += delta; }
  }

  int64_t sum(int64_t x) {
    int64_t ret = 0;
    for (; x > 0; x -= lowbit(x)) { ret += tree[x]; }
    return ret;
  }

  int64_t range_sum(int64_t x, int64_t y) { return sum(y) - sum(x - 1); }

  static int64_t lowbit(int64_t x) { return x & (-x); }

  int64_t n;
  vector<int64_t> tree;
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, m;
  cin >> n >> m;

  // 修改操作: x 为位置, y = 1表示添加, -1表示删除, op = 0, v 为值
  // 查询操作: x 为左端点, y 为右端点, op = 1, v 为查询的第k小值, id 为查询编号
  struct Query {
    int64_t x, y, op, v, id;
  };

  int64_t max_value = 0;
  vector<int64_t> nums(n + 1);
  // 最多 n 个初始值, m 次修改(每次修改视作删除旧值和添加新值两次操作)
  vector<Query> queries(n + 2 * m + 1);
  int64_t op_id = 0, query_id = 0;
  for (int64_t i = 1; i <= n; ++i) {
    cin >> nums[i];
    queries[++op_id] = {.x = i, .y = 1, .op = 0, .v = nums[i], .id = 0};
    max_value        = max(max_value, nums[i]);
  }
  char type;
  for (int64_t i = 1; i <= m; ++i) {
    cin >> type;
    if (type == 'Q') {
      int64_t l, r, k;
      cin >> l >> r >> k;
      queries[++op_id] = {.x = l, .y = r, .op = 1, .v = k, .id = ++query_id};
    } else {
      int64_t pos, v;
      cin >> pos >> v;
      queries[++op_id] = {.x = pos, .y = -1, .op = 0, .v = nums[pos], .id = 0};  // 删除旧值
      queries[++op_id] = {.x = pos, .y = 1, .op = 0, .v = v, .id = 0};           // 添加新值
      nums[pos]        = v;
      max_value        = max(max_value, v);
    }
  }
  vector<int64_t> qid(op_id + 1);
  iota(qid.begin(), qid.end(), 0);

  vector<int64_t> left(op_id + 1);
  vector<int64_t> right(op_id + 1);

  vector<int64_t> answers(query_id + 1);

  BIT bit(n);

  auto dfs = [&](auto &&self, int64_t ql, int64_t qr, int64_t l, int64_t r) -> void {
    if (ql > qr) { return; }  // 无查询
    if (l == r) {
      for (int64_t i = ql; i <= qr; i++) {
        if (queries[qid[i]].op == 1) { answers[queries[qid[i]].id] = l; }
      }
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    // 分配查询
    int64_t left_count = 0, right_count = 0;
    for (int64_t i = ql; i <= qr; i++) {
      auto [x, y, op, v, id] = queries[qid[i]];
      if (op == 0) {  // 修改操作
        if (v <= mid) {
          bit.point_add(x, y);
          left[left_count++] = qid[i];
        } else {
          right[right_count++] = qid[i];
        }
      } else {  // 查询操作
        int64_t cnt = bit.range_sum(x, y);
        if (v <= cnt) {  // 在左半区间
          left[left_count++] = qid[i];
        } else {  // 在右半区间, 更新k值
          queries[qid[i]].v    -= cnt;
          right[right_count++]  = qid[i];
        }
      }
    }
    // 重新排列qid
    for (int64_t i = 0; i < left_count; i++) { qid[ql + i] = left[i]; }
    for (int64_t i = 0; i < right_count; i++) { qid[ql + i + left_count] = right[i]; }
    // 恢复BIT
    for (int64_t i = 0; i < left_count; i++) {
      auto [x, y, op, v, id] = queries[left[i]];
      if (op == 0 && v <= mid) { bit.point_add(x, -y); }
    }
    // 递归处理左右子区间
    self(self, ql, ql + left_count - 1, l, mid);
    self(self, ql + left_count, qr, mid + 1, r);
  };
  // 查询id从1到op_id, 答案范围从 0 到 max_value
  dfs(dfs, 1, op_id, 0, max_value);
  for (int64_t i = 1; i <= query_id; i++) { cout << answers[i] << "\n"; }

  return 0;
}
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <tuple>
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
  vector<int64_t> tree;  // one-based indexing
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n;
  cin >> n;
  BIT bit(n);
  vector<int64_t> nums(n + 1);
  for (int i = 1; i <= n; i++) { cin >> nums[i]; }

  vector<int64_t> sorted_nums = nums;
  sort(sorted_nums.begin() + 1, sorted_nums.end());
  auto get_rank = [&](int64_t x) {  // 返回x的排名，排名从1开始
    return lower_bound(sorted_nums.begin() + 1, sorted_nums.end(), x) - sorted_nums.begin();
  };

  bit = BIT(n);
  vector<int64_t> last(n + 1, 0);  // last[i]表示第i个数上次出现的位置

  using TIII = tuple<int64_t, int64_t, int64_t>;  // left, right, id

  int64_t m;
  cin >> m;
  vector<TIII> queries(m);
  for (int64_t i = 0; i < m; i++) {
    int64_t left, right;
    cin >> left >> right;
    queries[i] = make_tuple(left, right, i);
  }
  sort(queries.begin(), queries.end(),
       [](const TIII &a, const TIII &b) { return get<1>(a) < get<1>(b); });

  vector<int64_t> answers(m);
  int i = 1;
  for (auto [left, right, id] : queries) {
    // 处理每个查询
    for (; i <= right; i++) {
      int64_t rank = get_rank(nums[i]);
      // 取消上次出现
      if (last[rank] != 0) { bit.point_add(last[rank], -1); }
      bit.point_add(i, 1);  // 添加本次出现
      last[rank] = i;
    }
    answers[id] = bit.range_sum(left, right);
  }

  for (const auto &ans : answers) { cout << ans << '\n'; }

  return 0;
}
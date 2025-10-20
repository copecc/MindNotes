#include <algorithm>
#include <cstdint>
#include <iostream>
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
  int n;
  cin >> n;
  vector<int> nums(n);
  for (int i = 0; i < n; ++i) { cin >> nums[i]; }
  vector<int> sorted_nums = nums;
  sort(sorted_nums.begin(), sorted_nums.end());

  auto get_rank = [&](int num) {
    return lower_bound(sorted_nums.begin(), sorted_nums.end(), num) - sorted_nums.begin() + 1;
  };

  int64_t ans = 0;
  BIT bit(n);
  for (int64_t i = n - 1; i >= 0; --i) {
    int rank  = get_rank(nums[i]);
    ans      += bit.range_sum(1, rank - 1);
    bit.point_add(rank, 1);
  }
  cout << ans << '\n';
  return 0;
}
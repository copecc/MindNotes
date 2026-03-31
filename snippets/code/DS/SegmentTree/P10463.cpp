#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> sum;  // 区间和信息
  vector<int64_t> gcd;  // 区间gcd信息

  explicit segment_tree(int64_t n) : sum(n * 4), gcd(n * 4) {}

  void push_up(int64_t i) {
    sum[i] = sum[2 * i] + sum[2 * i + 1];
    gcd[i] = std::gcd(gcd[2 * i], gcd[2 * i + 1]);
  }

  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums) {
    if (left == right) {  // 叶子节点，差分数组的值
      sum[i] = gcd[i] = nums[left] - nums[left - 1];
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums);
    build(2 * i + 1, mid + 1, right, nums);
    push_up(i);
  }

  void point_add(int64_t index, int64_t val, int64_t i, int64_t left, int64_t right) {
    if (left == index && right == index) {  // 到叶子，直接修改数组中的值
      sum[i] += val;
      gcd[i] += val;
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    if (index <= mid) {  // 继续往下找
      point_add(index, val, 2 * i, left, mid);
    } else {
      point_add(index, val, 2 * i + 1, mid + 1, right);
    }
    push_up(i);  // 更新当前节点的值
  }

  int64_t range_sum(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return sum[i]; }
    int64_t mid = l + ((r - l) / 2);
    int64_t res = 0;
    if (ql <= mid) { res += range_sum(ql, qr, 2 * i, l, mid); }
    if (qr > mid) { res += range_sum(ql, qr, 2 * i + 1, mid + 1, r); }
    return res;
  }

  int64_t range_gcd(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return gcd[i]; }
    int64_t mid = l + ((r - l) / 2);
    int64_t res = 0;
    if (ql <= mid) { res = std::gcd(res, range_gcd(ql, qr, 2 * i, l, mid)); }
    if (qr > mid) { res = std::gcd(res, range_gcd(ql, qr, 2 * i + 1, mid + 1, r)); }
    return res;
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
  segment_tree seg(n);
  seg.build(1, 1, n, nums);
  for (int64_t i = 0; i < m; ++i) {
    char op;
    cin >> op;
    if (op == 'Q') {
      int64_t left, right;
      cin >> left >> right;
      int64_t a_left    = seg.range_sum(1, left, 1, 1, n);
      int64_t range_gcd = seg.range_gcd(left + 1, right, 1, 1, n);
      cout << std::abs(std::gcd(a_left, range_gcd)) << '\n';
    } else if (op == 'C') {
      int64_t left, right, val;
      cin >> left >> right >> val;
      seg.point_add(left, val, 1, 1, n);
      if (right + 1 <= n) { seg.point_add(right + 1, -val, 1, 1, n); }
    }
  }
  return 0;
}
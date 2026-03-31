#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> sum;
  vector<int64_t> tag_add;

  explicit segment_tree(int64_t n) : sum(n * 4), tag_add(n * 4) {}

  void push_up(int64_t i) { sum[i] = sum[2 * i] + sum[2 * i + 1]; }

  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums) {
    if (left == right) {
      sum[i] = nums[left];
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums);
    build(2 * i + 1, mid + 1, right, nums);
    push_up(i);
  }

  void lazy_add(int64_t i, int64_t val, int64_t count) {
    sum[i]     += count * val;
    tag_add[i] += val;
  }

  void push_down(int64_t i, int64_t left_count, int64_t right_count) {
    if (tag_add[i] != 0) {
      lazy_add(2 * i, tag_add[i], left_count);
      lazy_add(2 * i + 1, tag_add[i], right_count);
      tag_add[i] = 0;
    }
  }

  void range_add(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) {
      lazy_add(i, val, r - l + 1);
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    if (ql <= mid) { range_add(ql, qr, val, 2 * i, l, mid); }
    if (qr > mid) { range_add(ql, qr, val, 2 * i + 1, mid + 1, r); }
    push_up(i);
  }

  int64_t range_sum(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return sum[i]; }
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    int64_t res = 0;
    if (ql <= mid) { res += range_sum(ql, qr, 2 * i, l, mid); }
    if (qr > mid) { res += range_sum(ql, qr, 2 * i + 1, mid + 1, r); }
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
  vector<int64_t> diff(n + 1);
  for (int64_t i = 1; i <= n; ++i) {
    cin >> nums[i];
    diff[i] = nums[i] - nums[i - 1];
  }
  segment_tree seg_tree(n);
  seg_tree.build(1, 1, n, diff);
  for (int64_t i = 0; i < m; ++i) {
    int64_t op;
    cin >> op;
    if (op == 1) {
      int64_t l, r, k, d;
      cin >> l >> r >> k >> d;
      // 加上等差数列, l位置加k, [l+1,r]区间加d, r+1位置减去k+(r-l)*d
      seg_tree.range_add(l, l, k, 1, 1, n);
      if (l + 1 <= r) { seg_tree.range_add(l + 1, r, d, 1, 1, n); }
      if (r + 1 <= n) {
        int64_t val = k + (r - l) * d;
        seg_tree.range_add(r + 1, r + 1, -val, 1, 1, n);
      }
    } else if (op == 2) {
      int64_t index;
      cin >> index;
      int64_t result = seg_tree.range_sum(1, index, 1, 1, n);
      cout << result << '\n';
    }
  }
  return 0;
}
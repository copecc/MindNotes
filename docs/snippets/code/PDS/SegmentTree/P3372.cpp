#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct tag_p_tree {
  vector<int64_t> sum;      // 区间和
  vector<int64_t> tag_add;  // 加法标记

  explicit tag_p_tree(int64_t n) : sum(n * 4), tag_add(n * 4) {}

  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums) {
    if (left == right) {  // 叶子节点，初始化为数组中对应值
      sum[i] = nums[left];
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums);
    build(2 * i + 1, mid + 1, right, nums);
    // 根据子树更新
    sum[i] = sum[2 * i] + sum[2 * i + 1];
  }

  // 区间加法
  void range_add(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    sum[i] += val * (min(qr, r) - max(ql, l) + 1);
    if (ql <= l && r <= qr) {  // 区间覆盖, 直接更新
      tag_add[i] += val;
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    if (ql <= mid) { range_add(ql, qr, val, 2 * i, l, mid); }
    if (qr > mid) { range_add(ql, qr, val, 2 * i + 1, mid + 1, r); }
  }

  // 查询区间和
  int64_t range_sum(int64_t ql, int64_t qr, int64_t added, int64_t i, int64_t l, int64_t r) {
    // 区间覆盖，直接返回
    if (ql <= l && r <= qr) { return sum[i] + added * (r - l + 1); }
    int64_t mid = l + ((r - l) / 2);
    // 汇总结果
    int64_t res = 0;
    if (ql <= mid) { res += range_sum(ql, qr, added + tag_add[i], 2 * i, l, mid); }
    if (qr > mid) { res += range_sum(ql, qr, added + tag_add[i], 2 * i + 1, mid + 1, r); }
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
  for (int64_t i = 1; i <= n; i++) { cin >> nums[i]; }
  tag_p_tree seg(n);
  seg.build(1, 1, n, nums);
  for (int64_t i = 0; i < m; i++) {
    int64_t op;
    cin >> op;
    if (op == 1) {
      int64_t l, r, val;
      cin >> l >> r >> val;
      seg.range_add(l, r, val, 1, 1, n);
    } else {
      int64_t l, r;
      cin >> l >> r;
      cout << seg.range_sum(l, r, 0, 1, 1, n) << '\n';
    }
  }
  return 0;
}
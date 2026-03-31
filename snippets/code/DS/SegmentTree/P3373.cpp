#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> sum;      // 区间和
  vector<int64_t> tag_add;  // 区间加法懒标记
  vector<int64_t> tag_mul;  // 区间乘法懒标记

  int64_t m;  // 取模

  explicit segment_tree(int64_t n, int64_t m)
      : sum(n * 4), tag_add(n * 4), tag_mul(n * 4, 1), m(m) {}

  void push_up(int64_t i) { sum[i] = (sum[2 * i] + sum[2 * i + 1]) % m; }

  // 构建线段树
  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums) {
    if (left == right) {  // 叶子节点，进行初始化
      sum[i] = nums[left] % m;
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums);
    build(2 * i + 1, mid + 1, right, nums);
    push_up(i);
  }

  void lazy_add(int64_t i, int64_t val, int64_t count) {
    sum[i]     = (sum[i] + count * val) % m;
    tag_add[i] = (tag_add[i] + val) % m;
  }

  void lazy_mul(int64_t i, int64_t val, int64_t count) {
    sum[i]     = (sum[i] * val) % m;
    tag_mul[i] = (tag_mul[i] * val) % m;
    tag_add[i] = (tag_add[i] * val) % m;  // 乘法会影响加法标记
  }

  // 向下传递懒标记
  void push_down(int64_t i, int64_t left_count, int64_t right_count) {
    if (tag_mul[i] != 1) {  // 将乘法标记传递给子节点
      lazy_mul(2 * i, tag_mul[i], left_count);
      lazy_mul(2 * i + 1, tag_mul[i], right_count);
      tag_mul[i] = 1;  // 清空根节点乘法标记
    }
    if (tag_add[i] != 0) {  // 将加法标记传递给子节点
      lazy_add(2 * i, tag_add[i], left_count);
      lazy_add(2 * i + 1, tag_add[i], right_count);
      tag_add[i] = 0;  // 清空根节点加法标记
    }
  }

  // 区间加法: range_add(x, y, val, 1, 1, n) 将区间 [x,y] 的值加上 val
  void range_add(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) {  // 区间覆盖, 直接更新
      lazy_add(i, val, r - l + 1);
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    if (ql <= mid) { range_add(ql, qr, val, 2 * i, l, mid); }
    if (qr > mid) { range_add(ql, qr, val, 2 * i + 1, mid + 1, r); }
    push_up(i);
  }

  // 区间乘法: range_mul(x, y, val, 1, 1, n) 将区间 [x,y] 的值乘以 val
  void range_mul(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) {  // 区间覆盖, 直接更新
      lazy_mul(i, val, r - l + 1);
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    if (ql <= mid) { range_mul(ql, qr, val, 2 * i, l, mid); }
    if (qr > mid) { range_mul(ql, qr, val, 2 * i + 1, mid + 1, r); }
    push_up(i);
  }

  // 区间求和: range_sum(x, y, 1, 1, n) 查询区间 [x,y] 的和
  int64_t range_sum(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return sum[i]; }  // 区间覆盖，直接返回
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    // 汇总结果
    int64_t res = 0;
    if (ql <= mid) { res = (res + range_sum(ql, qr, 2 * i, l, mid)) % m; }
    if (qr > mid) { res = (res + range_sum(ql, qr, 2 * i + 1, mid + 1, r)) % m; }
    return res;
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int64_t n, q, m;
  cin >> n >> q >> m;
  vector<int64_t> nums(n + 1);
  for (int64_t i = 1; i <= n; i++) { cin >> nums[i]; }
  segment_tree seg(n, m);
  seg.build(1, 1, n, nums);
  for (int64_t i = 0; i < q; i++) {
    int64_t op;
    cin >> op;
    if (op == 1) {  // 区间乘法
      int64_t l, r, x;
      cin >> l >> r >> x;
      seg.range_mul(l, r, x, 1, 1, n);
    } else if (op == 2) {  // 区间加法
      int64_t l, r, x;
      cin >> l >> r >> x;
      seg.range_add(l, r, x, 1, 1, n);
    } else if (op == 3) {  // 区间求和
      int64_t l, r;
      cin >> l >> r;
      cout << seg.range_sum(l, r, 1, 1, n) << '\n';
    }
  }
  return 0;
}
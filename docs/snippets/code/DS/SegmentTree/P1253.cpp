#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> sum;          // 区间和
  vector<int64_t> maximum;      // 区间最大值
  vector<int64_t> tag_add;      // 区间加法懒标记
  vector<int64_t> tag_set;      // 区间赋值懒标记
  vector<int64_t> tag_set_val;  // 区间赋值懒标记值, 只有tag_set为true时该值才有意义

  explicit segment_tree(int64_t n)
      : sum(n * 4), maximum(n * 4), tag_add(n * 4), tag_set(n * 4), tag_set_val(n * 4) {}

  void push_up(int64_t i) {
    sum[i]     = sum[2 * i] + sum[2 * i + 1];
    maximum[i] = max(maximum[2 * i], maximum[2 * i + 1]);
  }

  // 构建线段树
  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums) {
    if (left == right) {  // 叶子节点，进行初始化
      sum[i] = maximum[i] = nums[left];
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums);
    build(2 * i + 1, mid + 1, right, nums);
    push_up(i);
  }

  void lazy_add(int64_t i, int64_t val, int64_t count) {
    sum[i]     += count * val;
    maximum[i] += val;
    tag_add[i] += val;
  }

  void lazy_set(int64_t i, int64_t val, int64_t count) {
    sum[i]         = count * val;
    maximum[i]     = val;
    tag_set[i]     = 1;
    tag_set_val[i] = val;
    tag_add[i]     = 0;  // 清空加法标记
  }

  // 向下传递懒标记
  void push_down(int64_t i, int64_t left_count, int64_t right_count) {
    if (tag_set[i] != 0) {  // 处理赋值
      lazy_set(2 * i, tag_set_val[i], left_count);
      lazy_set(2 * i + 1, tag_set_val[i], right_count);
      tag_set[i] = 0;  // 清空根节点赋值标记
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

  // 区间赋值: range_set(x, y, val, 1, 1, n) 将区间 [x,y] 的值修改为 val
  void range_set(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) {  // 区间覆盖, 直接更新
      lazy_set(i, val, r - l + 1);
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    if (ql <= mid) { range_set(ql, qr, val, 2 * i, l, mid); }
    if (qr > mid) { range_set(ql, qr, val, 2 * i + 1, mid + 1, r); }
    push_up(i);
  }

  // 区间求和: range_sum(x, y, 1, 1, n) 查询区间 [x,y] 的和
  int64_t range_sum(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return sum[i]; }  // 区间覆盖，直接返回
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    // 汇总结果
    int64_t res = 0;
    if (ql <= mid) { res += range_sum(ql, qr, 2 * i, l, mid); }
    if (qr > mid) { res += range_sum(ql, qr, 2 * i + 1, mid + 1, r); }
    return res;
  }

  // 区间求最大值: range_max(x, y, 1, 1, n) 查询区间 [x,y] 的最大值
  int64_t range_max(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return maximum[i]; }  // 区间覆盖，直接返回
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    // 汇总结果
    int64_t res = INT64_MIN;
    if (ql <= mid) { res = max(res, range_max(ql, qr, 2 * i, l, mid)); }
    if (qr > mid) { res = max(res, range_max(ql, qr, 2 * i + 1, mid + 1, r)); }
    return res;
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int64_t n, q;
  cin >> n >> q;
  vector<int64_t> nums(n + 1);
  for (int64_t i = 1; i <= n; i++) { cin >> nums[i]; }
  segment_tree seg(n);
  seg.build(1, 1, n, nums);
  for (int64_t i = 0; i < q; i++) {
    int64_t op;
    cin >> op;
    if (op == 1) {  // 区间赋值
      int64_t l, r, x;
      cin >> l >> r >> x;
      seg.range_set(l, r, x, 1, 1, n);
    } else if (op == 2) {  // 区间加法
      int64_t l, r, x;
      cin >> l >> r >> x;
      seg.range_add(l, r, x, 1, 1, n);
    } else if (op == 3) {  // 区间最大值
      int64_t l, r;
      cin >> l >> r;
      cout << seg.range_max(l, r, 1, 1, n) << '\n';
    }
  }
  return 0;
}
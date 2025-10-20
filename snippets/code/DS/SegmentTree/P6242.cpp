#include <algorithm>
#include <cstdint>
#include <ios>
#include <iostream>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> sum;          // 区间和
  vector<int64_t> maximum;      // 区间最大值
  vector<int64_t> max_count;    // 区间内最大值的个数
  vector<int64_t> second_max;   // 区间内严格第二大值
  vector<int64_t> history_max;  // 区间内历史最大值

  vector<int64_t> max_add;    // 最大值的加法懒惰标记
  vector<int64_t> other_add;  // 非最大值的加法
  vector<int64_t> max_up;     // 最大值达到过的最大涨幅
  vector<int64_t> other_up;   // 非最大值达到过的最大涨幅

  explicit segment_tree(int64_t n)
      : sum(n * 4),
        maximum(n * 4),
        max_count(n * 4),
        second_max(n * 4),
        history_max(n * 4),
        max_add(n * 4),
        other_add(n * 4),
        max_up(n * 4),
        other_up(n * 4) {}

  void push_up(int64_t i) {
    sum[i]         = sum[2 * i] + sum[2 * i + 1];
    maximum[i]     = max(maximum[2 * i], maximum[2 * i + 1]);
    history_max[i] = max(history_max[2 * i], history_max[2 * i + 1]);

    if (maximum[2 * i] > maximum[2 * i + 1]) {  // 左子树最大值更大
      max_count[i]  = max_count[2 * i];
      second_max[i] = max(second_max[2 * i], maximum[2 * i + 1]);
    } else if (maximum[2 * i] < maximum[2 * i + 1]) {  // 右子树最大值更大
      max_count[i]  = max_count[2 * i + 1];
      second_max[i] = max(second_max[2 * i + 1], maximum[2 * i]);
    } else {  // 两个子树最大值相等
      max_count[i]  = max_count[2 * i] + max_count[2 * i + 1];
      second_max[i] = max(second_max[2 * i], second_max[2 * i + 1]);
    }
  }

  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums) {
    if (left == right) {
      sum[i] = maximum[i] = history_max[i] = nums[left];
      max_count[i]                         = 1;
      second_max[i]                        = INT64_MIN;  // 只有一个元素没有第二大值
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums);
    build(2 * i + 1, mid + 1, right, nums);
    push_up(i);
  }

  // max_add_val: 最大值的加法更新值, other_add_val: 非最大值的加法更新值
  // max_up: 最大值的涨幅, other_up: 非最大值的涨幅
  void lazy_add(int64_t i, int64_t count, int64_t max_add_val, int64_t other_add_val,
                int64_t max_up_val, int64_t other_up_val) {
    // 首先维护区间历史最大值
    history_max[i] = max(history_max[i], maximum[i] + max_up_val);
    max_up[i]      = max(max_up[i], max_add[i] + max_up_val);
    other_up[i]    = max(other_up[i], other_add[i] + other_up_val);
    // 维护节点值
    sum[i]        += max_add_val * max_count[i] + other_add_val * (count - max_count[i]);
    maximum[i]    += max_add_val;
    second_max[i] += second_max[i] == INT64_MIN ? 0 : other_add_val;
    // 维护懒惰标记
    max_add[i]   += max_add_val;
    other_add[i] += other_add_val;
  }

  void push_down(int64_t i, int64_t left_count, int64_t right_count) {
    int64_t max_val = max(maximum[2 * i], maximum[2 * i + 1]);

    if (maximum[2 * i] == max_val) {  // 左子树最大值等于当前节点最大值
      lazy_add(2 * i, left_count, max_add[i], other_add[i], max_up[i], other_up[i]);
    } else {  // 左子树最大值小于当前节点最大值
      lazy_add(2 * i, left_count, other_add[i], other_add[i], other_up[i], other_up[i]);
    }

    if (maximum[2 * i + 1] == max_val) {  // 右子树最大值等于当前节点最大值
      lazy_add(2 * i + 1, right_count, max_add[i], other_add[i], max_up[i], other_up[i]);
    } else {  // 右子树最大值小于当前节点最大值
      lazy_add(2 * i + 1, right_count, other_add[i], other_add[i], other_up[i], other_up[i]);
    }
    // 清空根节点的加法标记
    max_add[i] = other_add[i] = 0;
    max_up[i] = other_up[i] = 0;
  }

  void range_add(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) {
      lazy_add(i, r - l + 1, val, val, val, val);
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    if (ql <= mid) { range_add(ql, qr, val, 2 * i, l, mid); }
    if (qr > mid) { range_add(ql, qr, val, 2 * i + 1, mid + 1, r); }
    push_up(i);
  }

  // 区间取min操作, 如果当前节点值大于等于val则更新为val, 否则不更新
  void range_set_min(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    if (val >= maximum[i]) { return; }  // 当前节点值已经小于等于val, 不需要更新

    if (ql <= l && r <= qr) {
      if (val > second_max[i]) {  // 只会影响最大值, 不会影响第二大值, 直接更新返回
        lazy_add(i, r - l + 1, val - maximum[i], 0, val - maximum[i], 0);
        return;
      }
      // 需要影响非最大值, 继续向下传递
    }

    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    if (ql <= mid) { range_set_min(ql, qr, val, 2 * i, l, mid); }
    if (qr > mid) { range_set_min(ql, qr, val, 2 * i + 1, mid + 1, r); }
    push_up(i);
  }

  // 查询区间和: range_sum(x, y, 1, 1, n) 表示查询区间 [x,y] 的和
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

  // 查询区间最大值: range_max(x, y, 1, 1, n) 查询区间 [x,y] 的最大值
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

  // 查询区间历史最大值: range_history_max(x, y, 1, 1, n) 查询区间 [x,y]
  // 的历史最大值
  int64_t range_history_max(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return history_max[i]; }
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    // 分割区间
    int64_t res = INT64_MIN;
    if (ql <= mid) { res = max(res, range_history_max(ql, qr, 2 * i, l, mid)); }
    if (qr > mid) { res = max(res, range_history_max(ql, qr, 2 * i + 1, mid + 1, r)); }
    return res;
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n, m;
  cin >> n >> m;
  vector<int64_t> nums(n + 1);
  for (int i = 1; i <= n; i++) { cin >> nums[i]; }
  segment_tree seg(n);
  seg.build(1, 1, n, nums);

  for (int i = 0; i < m; i++) {
    int op;
    cin >> op;
    if (op == 1) {
      int l, r, k;
      cin >> l >> r >> k;
      seg.range_add(l, r, k, 1, 1, n);
    } else if (op == 2) {
      int l, r, v;
      cin >> l >> r >> v;
      seg.range_set_min(l, r, v, 1, 1, n);
    } else if (op == 3) {
      int l, r;
      cin >> l >> r;
      cout << seg.range_sum(l, r, 1, 1, n) << '\n';
    } else if (op == 4) {
      int l, r;
      cin >> l >> r;
      cout << seg.range_max(l, r, 1, 1, n) << '\n';
    } else if (op == 5) {
      int l, r;
      cin >> l >> r;
      cout << seg.range_history_max(l, r, 1, 1, n) << '\n';
    }
  }
  return 0;
}
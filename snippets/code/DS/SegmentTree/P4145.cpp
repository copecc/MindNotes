#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> sum;      // 区间和
  vector<int64_t> maximum;  // 区间最大值

  explicit segment_tree(int64_t n) : sum(n * 4), maximum(n * 4) {}

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

  // 区间开方: range_sqrt(x, y, 1, 1, n) 将区间 [x,y] 的值开方
  void range_sqrt(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (l == r) {  // 叶子节点，进行开方
      auto val   = static_cast<int64_t>(sqrt(sum[i]));
      sum[i]     = val;
      maximum[i] = val;
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    // 只对有可能变化的子节点进行递归
    if (ql <= mid && maximum[2 * i] > 1) { range_sqrt(ql, qr, 2 * i, l, mid); }
    if (qr > mid && maximum[2 * i + 1] > 1) { range_sqrt(ql, qr, 2 * i + 1, mid + 1, r); }
    push_up(i);
  }

  // 区间求和: range_sum(x, y, 1, 1, n) 查询区间 [x,y] 的和
  int64_t range_sum(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return sum[i]; }  // 区间覆盖，直接返回
    int64_t mid = l + ((r - l) / 2);
    // 汇总结果
    int64_t res = 0;
    if (ql <= mid) { res += range_sum(ql, qr, 2 * i, l, mid); }
    if (qr > mid) { res += range_sum(ql, qr, 2 * i + 1, mid + 1, r); }
    return res;
  }
};

int main() {
  int n;
  cin >> n;
  vector<int64_t> nums(n + 1);
  for (int i = 1; i <= n; ++i) { cin >> nums[i]; }
  segment_tree seg(n);
  seg.build(1, 1, n, nums);

  int m;
  cin >> m;
  for (int i = 0; i < m; ++i) {
    int op, l, r;
    cin >> op >> l >> r;
    if (l > r) { swap(l, r); }
    if (op == 0) {
      seg.range_sqrt(l, r, 1, 1, n);
    } else if (op == 1) {
      cout << seg.range_sum(l, r, 1, 1, n) << '\n';
    }
  }
}
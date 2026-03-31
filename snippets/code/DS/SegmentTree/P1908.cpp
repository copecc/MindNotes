#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> sum;  // 区间和

  explicit segment_tree(int64_t n) : sum(n * 4) {}

  void push_up(int64_t i) { sum[i] = sum[2 * i] + sum[2 * i + 1]; }

  // 单点修改: point_add(x, val, 1, 1, n) 将下标 x 的值加上 val
  void point_add(int64_t index, int64_t val, int64_t i, int64_t left, int64_t right) {
    if (left == index && right == index) {  // 到叶子，直接修改数组中的值
      sum[i] += val;
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

  // 区间求和: range_sum(x, y, 1, 1, n) 查询区间 [x,y] 的和
  int64_t range_sum(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql > r || qr < l) { return 0; }         // 无覆盖
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
  vector<int> nums(n);
  for (int i = 0; i < n; ++i) { cin >> nums[i]; }
  vector<int> sorted_nums = nums;
  sort(sorted_nums.begin(), sorted_nums.end());

  auto get_rank = [&](int num) {
    return lower_bound(sorted_nums.begin(), sorted_nums.end(), num) - sorted_nums.begin() + 1;
  };

  int64_t ans = 0;
  segment_tree seg(n + 1);
  for (int64_t i = n - 1; i >= 0; --i) {
    int rank  = get_rank(nums[i]);
    ans      += seg.range_sum(1, rank - 1, 1, 1, n);
    seg.point_add(rank, 1, 1, 1, n);
  }
  cout << ans << '\n';
  return 0;
}
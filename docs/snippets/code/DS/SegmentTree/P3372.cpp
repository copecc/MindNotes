#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct segment_tree {       // (1)!
  vector<int64_t> sum;      // 区间和 (2)
  vector<int64_t> tag_add;  // 区间加法懒标记

  explicit segment_tree(int64_t n) : sum(n * 4), tag_add(n * 4) {}

  void push_up(int64_t i) { sum[i] = sum[2 * i] + sum[2 * i + 1]; }  // (3)!

  // 构建线段树
  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums) {
    if (left == right) {  // 叶子节点，进行初始化
      sum[i] = nums[left];
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums);
    build(2 * i + 1, mid + 1, right, nums);
    push_up(i);
  }

  // 单点修改: point_set(x, val, 1, 1, n) 将下标 x 的值修改为 val (4)
  void point_set(int64_t index, int64_t val, int64_t i, int64_t left, int64_t right) {
    if (left == index && right == index) {  // 到叶子，直接修改数组中的值
      sum[i] = val;
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    if (index <= mid) {  // 继续往下找
      point_set(index, val, 2 * i, left, mid);
    } else {
      point_set(index, val, 2 * i + 1, mid + 1, right);
    }
    push_up(i);  // 更新当前节点的值
  }

  void lazy_add(int64_t i, int64_t val, int64_t count) {
    sum[i]     += count * val;
    tag_add[i] += val;
  }

  // 向下传递懒标记
  void push_down(int64_t i, int64_t left_count, int64_t right_count) {
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
};

int main() {
  int64_t n, m;
  cin >> n >> m;
  vector<int64_t> nums(n + 1);
  for (int64_t i = 1; i <= n; i++) { cin >> nums[i]; }
  segment_tree seg(n);
  seg.build(1, 1, n, nums);
  for (int64_t i = 0; i < m; i++) {
    int64_t op;
    cin >> op;
    if (op == 1) {  // 区间加法
      int64_t l, r, val;
      cin >> l >> r >> val;
      seg.range_add(l, r, val, 1, 1, n);
    } else {  // 区间求和
      int64_t l, r;
      cin >> l >> r;
      cout << seg.range_sum(l, r, 1, 1, n) << '\n';
    }
  }
  return 0;
}
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> maximum;  // 区间最大值

  explicit segment_tree(int64_t n) : maximum(n * 4) {}

  // 构建线段树
  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums) {
    if (left == right) {  // 叶子节点，进行初始化
      maximum[i] = nums[left];
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums);
    build(2 * i + 1, mid + 1, right, nums);
    maximum[i] = max(maximum[2 * i], maximum[2 * i + 1]);
  }

  int64_t query(int64_t ql, int64_t qr, int64_t val, int64_t i, int64_t l, int64_t r) {
    if (maximum[i] <= val) { return -1; }
    if (l == r) { return l; }
    if (ql <= l && r <= qr) {  // 当前区间完全包含在查询区间内, 分别查询左右子树
      if (maximum[2 * i] > val) { return query(ql, qr, val, 2 * i, l, (l + r) / 2); }
      return query(ql, qr, val, 2 * i + 1, (l + r) / 2 + 1, r);
    }
    int64_t mid = l + ((r - l) / 2);
    int64_t res = -1;
    // 左子树有交集
    if (ql <= mid) {
      res = query(ql, qr, val, 2 * i, l, mid);
      if (res != -1) { return res; }  // 左子树找到答案，直接返回
    }
    // 右子树有交集
    if (qr > mid) { res = query(ql, qr, val, 2 * i + 1, mid + 1, r); }
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
  while ((m--) != 0) {
    int64_t l, r, x;
    cin >> l >> r >> x;
    cout << seg.query(l, r, x, 1, 1, n) << '\n';
  }
  return 0;
}
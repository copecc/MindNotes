#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> sum;          // 区间和
  vector<int64_t> max_seq;      // 区间最大子序列和
  vector<int64_t> max_prefix;   // 区间最大前缀和
  vector<int64_t> max_suffix;   // 区间最大后缀和
  vector<int64_t> tag_set;      // 区间赋值懒标记
  vector<int64_t> tag_set_val;  // 区间赋值懒标记值, 只有tag_set为true时该值才有意义

  explicit segment_tree(int64_t n)
      : sum(n * 4),
        max_seq(n * 4),
        max_prefix(n * 4),
        max_suffix(n * 4),
        tag_set(n * 4),
        tag_set_val(n * 4) {}

  void push_up(int64_t i) {
    sum[i] = sum[2 * i] + sum[2 * i + 1];
    // 最大子序列和相关, 需要考虑跨越左右子树的情况
    max_prefix[i] = max(max_prefix[2 * i], sum[2 * i] + max_prefix[2 * i + 1]);
    max_suffix[i] = max(max_suffix[2 * i + 1], sum[2 * i + 1] + max_suffix[2 * i]);
    max_seq[i]
        = max({max_seq[2 * i], max_seq[2 * i + 1], max_suffix[2 * i] + max_prefix[2 * i + 1]});
  }

  // 构建线段树
  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums) {
    if (left == right) {  // 叶子节点，进行初始化
      // 包含自身初始化为val, 否则初始化为max(0, val)
      sum[i] = max_seq[i] = max_prefix[i] = max_suffix[i] = nums[left];
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums);
    build(2 * i + 1, mid + 1, right, nums);
    push_up(i);
  }

  void lazy_set(int64_t i, int64_t val, int64_t count) {
    sum[i] = count * val;
    // 若val为负数, 则最大子序列和, 前缀和, 后缀和均取val, 否则取 val*count
    max_seq[i] = max_prefix[i] = max_suffix[i] = max(val, val * count);
    tag_set[i]                                 = 1;
    tag_set_val[i]                             = val;
  }

  // 向下传递懒标记
  void push_down(int64_t i, int64_t left_count, int64_t right_count) {
    if (tag_set[i] != 0) {  // 处理赋值
      lazy_set(2 * i, tag_set_val[i], left_count);
      lazy_set(2 * i + 1, tag_set_val[i], right_count);
      tag_set[i] = 0;  // 清空根节点赋值标记
    }
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

  struct Node {
    int64_t sum, max_seq, max_prefix, max_suffix;
  };

  // 区间最大子序列和: range_maxseq(x, y, 1, 1, n) 查询区间 [x,y] 的最大子序列和
  Node range_maxseq(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) {  // 区间覆盖，直接返回
      return {.sum        = sum[i],
              .max_seq    = max_seq[i],
              .max_prefix = max_prefix[i],
              .max_suffix = max_suffix[i]};
    }
    int64_t mid = l + ((r - l) / 2);
    push_down(i, mid - l + 1, r - mid);
    // 查询区间完全在左子树或右子树
    if (qr <= mid) { return range_maxseq(ql, qr, 2 * i, l, mid); }
    if (ql > mid) { return range_maxseq(ql, qr, 2 * i + 1, mid + 1, r); }
    // 查询区间跨越左右子树, 需要合并结果
    Node left_res  = range_maxseq(ql, qr, 2 * i, l, mid);
    Node right_res = range_maxseq(ql, qr, 2 * i + 1, mid + 1, r);
    Node res;
    res.sum        = left_res.sum + right_res.sum;  // 用于更新父节点的前缀和后缀和
    res.max_prefix = max(left_res.max_prefix, left_res.sum + right_res.max_prefix);
    res.max_suffix = max(right_res.max_suffix, right_res.sum + left_res.max_suffix);
    res.max_seq
        = max({left_res.max_seq, right_res.max_seq, left_res.max_suffix + right_res.max_prefix});
    return res;
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int64_t n;
  cin >> n;
  vector<int64_t> nums(n + 1);
  for (int64_t i = 1; i <= n; i++) { cin >> nums[i]; }
  segment_tree seg(n);
  seg.build(1, 1, n, nums);
  int64_t m;
  cin >> m;
  for (int64_t i = 0; i < m; i++) {
    int64_t op;
    cin >> op;
    if (op == 0) {  // 区间赋值
      int64_t x, y;
      cin >> x >> y;
      seg.range_set(x, x, y, 1, 1, n);
    } else {  // 区间最大子序列和
      int64_t l, r;
      cin >> l >> r;
      cout << seg.range_maxseq(l, r, 1, 1, n).max_seq << '\n';
    }
  }

  return 0;
}
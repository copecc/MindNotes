#include <unordered_map>
#include <vector>
using namespace std;

class Solution {
  struct segment_tree {
    vector<int> min;      // 区间最小值
    vector<int> max;      // 区间最大值
    vector<int> tag_add;  // 区间加法懒标记

    explicit segment_tree(int n) : min(n * 4), max(n * 4), tag_add(n * 4) {}

    void push_up(int i) {
      min[i] = std::min(min[2 * i], min[2 * i + 1]);
      max[i] = std::max(max[2 * i], max[2 * i + 1]);
    }

    void lazy_add(int i, int val, int count) {
      tag_add[i] += val;

      min[i]     += val;
      max[i]     += val;
    }

    // 向下传递懒标记
    void push_down(int i, int left_count, int right_count) {
      if (tag_add[i] != 0) {  // 将加法标记传递给子节点
        lazy_add(2 * i, tag_add[i], left_count);
        lazy_add(2 * i + 1, tag_add[i], right_count);
        tag_add[i] = 0;  // 清空根节点加法标记
      }
    }

    // 区间加法: range_add(x, y, val, 1, 1, n) 将区间 [x,y] 的值加上 val
    void range_add(int ql, int qr, int val, int i, int l, int r) {
      if (ql <= l && r <= qr) {  // 区间覆盖, 直接更新
        lazy_add(i, val, r - l + 1);
        return;
      }
      int mid = l + ((r - l) / 2);
      push_down(i, mid - l + 1, r - mid);
      if (ql <= mid) { range_add(ql, qr, val, 2 * i, l, mid); }
      if (qr > mid) { range_add(ql, qr, val, 2 * i + 1, mid + 1, r); }
      push_up(i);
    }

    // 查找区间内第一个等于 target 的位置, 找不到返回 -1
    int find_first(int ql, int qr, int target, int i, int l, int r) {
      if (l > qr || r < ql || target < min[i] || target > max[i]) {  // 无效区间
        return -1;
      }
      if (l == r) {  // 此处必然等于 target
        return l;
      }
      int mid = l + ((r - l) / 2);
      push_down(i, mid - l + 1, r - mid);
      int res = find_first(ql, qr, target, 2 * i, l, mid);
      if (res < 0) {  // 去右子树找
        res = find_first(ql, qr, target, 2 * i + 1, mid + 1, r);
      }
      return res;
    }
  };

 public:
  int longestBalanced(vector<int> &nums) {
    int n = nums.size();
    segment_tree seg_tree(n + 2);  // 前缀和，区间为 [1, n+1]
    unordered_map<int, int> last;
    int ans = 0, sum = 0;
    for (int i = 1; i <= n; ++i) {  // 1-based
      int x = nums[i - 1];
      int v = (x % 2 == 0) ? 1 : -1;
      if (!last.contains(x)) {
        sum += v;  // 维护前缀和
        // [i+1, n+1], 第i+1个位置表示前i个数的前缀和
        seg_tree.range_add(i + 1, n + 1, v, 1, 1, n + 1);
      } else {
        int pre = last[x];  // 上次出现位置
        // 取消上次出现的影响, 上一次x出现的位置影响了区间 [pre+1, n+1]
        // 当前x的位置影响了区间 [i+1, n+1]，把[pre+1, i]的值减去v
        seg_tree.range_add(pre + 1, i, -v, 1, 1, n + 1);
      }
      last[x] = i;
      // 查找前缀和等于sum的最左位置, 第i个位置表示前i-1个数的前缀和
      int pos = seg_tree.find_first(1, i, sum, 1, 1, n + 1);
      if (pos >= 1) { ans = std::max(ans, i - pos + 1); }
    }
    return ans;
  }
};
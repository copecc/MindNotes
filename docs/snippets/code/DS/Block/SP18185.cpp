#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int n;
  cin >> n;

  vector<int> nums(n + 1);
  for (int i = 1; i <= n; i++) { cin >> nums[i]; }
  vector<int> sorted = nums;
  // 预处理分块信息
  int block_len      = sqrt(n);
  int block_cnt      = (n + block_len - 1) / block_len;
  vector<int> block_id(n + 1);
  vector<int> block_left(block_cnt + 1);
  vector<int> block_right(block_cnt + 1);
  for (int i = 1; i <= n; i++) { block_id[i] = (i - 1) / block_len + 1; }
  // 对每块内的元素进行排序，方便后续查询
  auto *p = sorted.data();
  for (int i = 1; i <= block_cnt; i++) {
    block_left[i]  = (i - 1) * block_len + 1;
    block_right[i] = min(i * block_len, n);
    sort(p + block_left[i], p + block_right[i] + 1);
  }

  auto block_count = [&](int bid, int val) {
    return p + block_right[bid] + 1
         - lower_bound(p + block_left[bid], p + block_right[bid] + 1, val);
  };

  auto query = [&](int l, int r, int val) {
    int cnt      = 0;
    int left_id  = block_id[l];
    int right_id = block_id[r];
    if (left_id == right_id) {  // 同一块内直接计算
      for (int i = l; i <= r; i++) {
        if (nums[i] >= val) { cnt++; }
      }
    } else {
      for (int i = l; i <= block_right[left_id]; i++) {  // 计算左边界块
        if (nums[i] >= val) { cnt++; }
      }
      // 计算中间完整块
      for (int i = left_id + 1; i <= right_id - 1; i++) { cnt += block_count(i, val); }
      for (int i = block_left[right_id]; i <= r; i++) {  // 计算右边界块
        if (nums[i] >= val) { cnt++; }
      }
    }
    return cnt;
  };

  auto update = [&](int i, int v) {
    int b_id  = block_id[i];
    int left  = block_left[b_id];
    int right = block_right[b_id];
    nums[i]   = v;
    copy(nums.data() + left, nums.data() + right + 1, p + left);
    sort(p + left, p + right + 1);
  };

  int m;
  cin >> m;
  for (int _ = 0; _ < m; _++) {
    int op;
    cin >> op;
    if (op == 0) {
      int l, r, val;
      cin >> l >> r >> val;
      cout << query(l, r, val) << "\n";
    } else {
      int i, v;
      cin >> i >> v;
      update(i, v);
    }
  }

  return 0;
}
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int n, m;
  cin >> n >> m;

  vector<int> nums(n + 1);
  for (int i = 1; i <= n; i++) { cin >> nums[i]; }
  vector<int> sorted = nums;
  // 预处理分块信息
  int block_len = sqrt(n);
  int block_cnt = (n + block_len - 1) / block_len;
  vector<int> block_id(n + 1);
  vector<int> block_left(block_cnt + 1);
  vector<int> block_right(block_cnt + 1);
  vector<int> added(block_cnt + 1, 0);  // 用于记录每块的增量
  for (int i = 1; i <= n; i++) { block_id[i] = (i - 1) / block_len + 1; }
  // 对每块内的元素进行排序，方便后续查询
  auto *p = sorted.data();
  for (int i = 1; i <= block_cnt; i++) {
    block_left[i]  = (i - 1) * block_len + 1;
    block_right[i] = min(i * block_len, n);
    sort(p + block_left[i], p + block_right[i] + 1);
  }

  auto block_count = [&](int bid, int val) {
    val -= added[bid];  // 减去当前块的增量
    return p + block_right[bid] + 1
         - lower_bound(p + block_left[bid], p + block_right[bid] + 1, val);
  };

  auto query = [&](int l, int r, int val) {
    int cnt      = 0;
    int left_id  = block_id[l];
    int right_id = block_id[r];
    if (left_id == right_id) {  // 同一块内直接计算
      for (int i = l; i <= r; i++) {
        if (nums[i] + added[left_id] >= val) { cnt++; }
      }
    } else {
      for (int i = l; i <= block_right[left_id]; i++) {  // 计算左边界块
        if (nums[i] + added[left_id] >= val) { cnt++; }
      }
      // 计算中间完整块
      for (int i = left_id + 1; i <= right_id - 1; i++) { cnt += block_count(i, val); }
      for (int i = block_left[right_id]; i <= r; i++) {  // 计算右边界块
        if (nums[i] + added[right_id] >= val) { cnt++; }
      }
    }
    return cnt;
  };
  // 同一块内直接更新
  auto inner_update = [&](int b_id, int l, int r, int val) {
    for (int i = l; i <= r; i++) { nums[i] += val; }
    copy(nums.data() + block_left[b_id], nums.data() + block_right[b_id] + 1, p + block_left[b_id]);
    sort(p + block_left[b_id], p + block_right[b_id] + 1);
  };

  auto update = [&](int l, int r, int val) {
    int left_id  = block_id[l];
    int right_id = block_id[r];
    if (left_id == right_id) {  // 同一块内直接计算
      inner_update(left_id, l, r, val);
    } else {
      inner_update(left_id, l, block_right[left_id], val);  // 更新左边界块
      // 计算中间完整块
      for (int i = left_id + 1; i <= right_id - 1; i++) { added[i] += val; }
      inner_update(right_id, block_left[right_id], r, val);  // 更新右边界块
    }
  };

  for (int _ = 0; _ < m; _++) {
    char op;
    int l, r, val;
    cin >> op >> l >> r >> val;
    if (op == 'A') {
      cout << query(l, r, val) << "\n";
    } else {
      update(l, r, val);
    }
  }

  return 0;
}
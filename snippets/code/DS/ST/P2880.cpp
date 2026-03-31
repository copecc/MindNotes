#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

struct ST {
  int n, m;
  vector<vector<int>> st_max;
  vector<vector<int>> st_min;

  explicit ST(const vector<int> &nums)
      : n(nums.size()),
        m(32 - __builtin_clz(n + 1)),
        st_max(n + 1, vector<int>(m)),
        st_min(n + 1, vector<int>(m)) {
    // 预处理 ST 表, O(nlogn)
    for (int i = 1; i <= n; i++) {
      st_max[i][0] = nums[i - 1];
      st_min[i][0] = nums[i - 1];
    }
    for (int j = 1; j < m; j++) {
      for (int i = 1; i + (1 << j) - 1 <= n; i++) {  // 确保不会越界
        st_max[i][j] = max(st_max[i][j - 1], st_max[i + (1 << (j - 1))][j - 1]);
        st_min[i][j] = min(st_min[i][j - 1], st_min[i + (1 << (j - 1))][j - 1]);
      }
    }
  }

  int query(int l, int r) {
    int j = 32 - __builtin_clz(r - l + 1) - 1;  // 计算不超过区间长度的最大2的幂次
    return max(st_max[l][j], st_max[r - (1 << j) + 1][j])
         - min(st_min[l][j], st_min[r - (1 << j) + 1][j]);
  }
};

int main() {
  int n, q;
  cin >> n >> q;
  vector<int> nums(n);
  for (int i = 0; i < n; i++) { cin >> nums[i]; }
  ST st(nums);
  while ((q--) != 0) {
    int l, r;
    cin >> l >> r;
    cout << st.query(l, r) << '\n';
  }
  return 0;
}
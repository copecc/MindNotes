#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int solve(const vector<int> &nums, int l1, int r1, int l2, int r2, int bit) {
  if (bit < 0 || l1 >= r1 || l2 >= r2) { return 0; }
  int m1 = l1;  // [l1, m1) 当前位为0, [m1, r1) 当前位为1
  while (m1 < r1 && ((nums[m1] >> bit) & 1) == 0) { m1++; }
  int m2 = l2;  // [l2, m2) 当前位为0, [m2, r2) 当前位为1
  while (m2 < r2 && ((nums[m2] >> bit) & 1) == 0) { m2++; }
  int ans = 0;
  // 左0右1 和 左1右0能产生 1, 肯定比不产生1的大
  if ((l1 < m1 && m2 < r2) || (m1 < r1 && l2 < m2)) {
    ans = (1 << bit);
    // 同一个区间划分, 两种情况等价
    if (m1 == m2) { return ans + solve(nums, l1, m1, m2, r2, bit - 1); }
    // 不同区间划分, 选择组合的最大值
    return ans + max(solve(nums, l1, m1, m2, r2, bit - 1), solve(nums, m1, r1, l2, m2, bit - 1));
  }
  // 不能产生1，递归同位组合
  if (l1 < m1 && l2 < m2) { ans = max(ans, solve(nums, l1, m1, l2, m2, bit - 1)); }
  if (m1 < r1 && m2 < r2) { ans = max(ans, solve(nums, m1, r1, m2, r2, bit - 1)); }
  return ans;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n;
  cin >> n;
  vector<int> nums(n);
  for (int i = 0; i < n; ++i) { cin >> nums[i]; }
  // 先排序，使得相同bit划分后能原地递归
  sort(nums.begin(), nums.end());
  cout << solve(nums, 0, n, 0, n, 30) << "\n";
  return 0;
}
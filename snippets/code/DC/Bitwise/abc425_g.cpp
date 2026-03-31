#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int64_t dfs(const vector<int> &arr, int64_t m, int bit) {
  if (bit < 0) { return 0; }
  vector<int> a0, a1;  // a0: 当前位为0的数, a1: 当前位为1的数
  for (int x : arr) {
    if (((x >> bit) & 1) != 0) {
      a1.push_back(x);
    } else {
      a0.push_back(x);
    }
  }
  // 将 m 分成两部分: [0, half), [half, m)
  int64_t half = 1LL << bit;
  if (m == (1LL << (bit + 1))) {       // 完整对称区间
    if (!a0.empty() && !a1.empty()) {  // (1)!
      return dfs(a0, half, bit - 1) + dfs(a1, half, bit - 1);
    }
    return 2 * dfs(arr, half, bit - 1) + half * half;  // (2)!
  }
  int64_t res = 0;
  // [0, half) 处理左半部分
  if (!a0.empty()) {  // (3)!
    res += dfs(a0, min(m, half), bit - 1);
  } else {  // (4)!
    res += dfs(arr, min(m, half), bit - 1) + half * min(m, half);
  }
  // [half, m) 处理右半部分
  if (m > half) {       // 对应区间变为 [0, m - half)
    if (!a1.empty()) {  // (5)!
      res += dfs(a1, m - half, bit - 1);
    } else {  // (6)!
      res += dfs(arr, m - half, bit - 1) + half * (m - half);
    }
  }
  return res;
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n, m;
  cin >> n >> m;
  vector<int> nums(n);
  for (int i = 0; i < n; ++i) { cin >> nums[i]; }

  cout << dfs(nums, m, 30) << "\n";
  return 0;
}
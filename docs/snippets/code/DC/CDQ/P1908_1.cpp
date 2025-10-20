#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n;
  cin >> n;
  vector<int> nums(n);
  for (int i = 0; i < n; ++i) { cin >> nums[i]; }
  auto solve = [&](auto &&self, int low, int high) -> int64_t {
    if (low >= high) { return 0; }
    int mid = low + ((high - low) / 2);
    // 先递归计算左右子数组的逆序对数量
    int64_t count = self(self, low, mid) + self(self, mid + 1, high);
    vector<int> merge(high - low + 1);
    // 计算跨越左右子数组的逆序对数量并合并, 并且排序
    int i = low, j = mid + 1, k = 0;
    while (i <= mid && j <= high) {
      if (nums[i] <= nums[j]) {
        merge[k++] = nums[i++];
      } else {
        merge[k++]  = nums[j++];
        count      += mid - i + 1;  // 逆序对
      }
    }
    while (i <= mid) { merge[k++] = nums[i++]; }
    while (j <= high) { merge[k++] = nums[j++]; }
    for (int p = 0; p < merge.size(); ++p) { nums[low + p] = merge[p]; }
    return count;
  };

  int64_t ans = solve(solve, 0, n - 1);
  cout << ans << '\n';
  return 0;
}
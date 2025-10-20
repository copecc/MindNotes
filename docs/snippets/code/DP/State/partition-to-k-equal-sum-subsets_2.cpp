#include <numeric>
#include <vector>
using namespace std;

class Solution {
 public:
  bool canPartitionKSubsets(vector<int> &nums, int k) {
    int n   = nums.size();
    int sum = accumulate(nums.begin(), nums.end(), 0);
    if (sum % k != 0) return false;
    int target = sum / k;
    vector<int> dp(1 << n, -1);
    dp[0] = 0;  // 初始状态，未选择任何元素时，当前子集和为 0

    for (int mask = 0; mask < (1 << n); ++mask) {
      if (dp[mask] == -1) { continue; }  // 跳过不可达状态
      for (int i = 0; i < n; ++i) {
        if ((mask & (1 << i)) != 0) { continue; }  // 元素已被选择
        int next_sum = dp[mask] + nums[i];
        if (next_sum <= target) {
          dp[mask | (1 << i)] = next_sum % target;  // 更新状态
        }
      }
    }
    return dp[(1 << n) - 1] == 0;  // 检查是否所有元素都被选择且子集和为目标值
  }
};
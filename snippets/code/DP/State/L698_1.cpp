#include <functional>
#include <numeric>
#include <vector>
using namespace std;

class Solution {
 public:
  bool canPartitionKSubsets(vector<int> &nums, int k) {
    int sum = accumulate(nums.begin(), nums.end(), 0);
    if (sum % k != 0) { return false; }  // 总和不能被k整除
    int target = sum / k;
    int n      = nums.size();

    // 状态压缩dp, dp[status]表示状态status是否可行
    vector<int> dp(1 << n, -1);
    // status: 当前选择的数字集合, target: 每个子集的目标和, sum: 当前子集的和,
    // rest_k: 剩余子集数量
    function<bool(int, int, int)> dfs = [&](int status, int sum, int rest_k) {
      // 所有数字都被选择完, 且正好划分为k个子集
      if (rest_k == 0) { return status == 0; }
      if (dp[status] != -1) { return dp[status] == 1; }  // 记忆化
      bool ok = false;                                   // 当前状态是否可行
      // 尝试选择一个数字加入当前子集
      for (int i = 0; i < n; ++i) {
        // 数字i已经被选择
        if ((status & (1 << i)) == 0) { continue; }
        // 如果加入数字i后超过目标和, 则跳过
        if (sum + nums[i] > target) { continue; }
        int next_status = status ^ (1 << i);
        if (sum + nums[i] == target) {  // 当前子集已满, 开始下一个子集
          ok = dfs(next_status, 0, rest_k - 1);
        } else {  // 继续填充当前子集
          ok = dfs(next_status, sum + nums[i], rest_k);
        }
        if (ok) { break; }  // 找到一个可行解即可
      }
      dp[status] = ok ? 1 : 0;
      return ok;
    };
    return dfs((1 << n) - 1, 0, k);
  }
};
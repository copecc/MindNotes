#include <numeric>
#include <vector>
using namespace std;

class Solution {
 public:
  bool canPartition(vector<int>& nums) {
    int sum = accumulate(nums.begin(), nums.end(), 0);
    if (sum % 2 != 0) {
      return false;
    }

    int target = sum / 2;
    vector<char> dp(target + 1, false);
    dp[0] = true;
    for (int num : nums) {
      for (int j = target; j >= num; --j) {
        dp[j] = dp[j] || dp[j - num];
      }
    }
    return dp[target];
  }
};

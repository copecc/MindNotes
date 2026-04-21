#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<vector<int>> threeSum(vector<int>& nums) {
    sort(nums.begin(), nums.end());
    vector<vector<int>> ans;
    const int n = static_cast<int>(nums.size());

    for (int i = 0; i < n; ++i) {
      if (nums[i] > 0) {
        break;
      }
      if (i > 0 && nums[i] == nums[i - 1]) {
        continue;
      }

      int left = i + 1, right = n - 1;
      while (left < right) {
        int sum = nums[i] + nums[left] + nums[right];
        if (sum < 0) {
          ++left;
        } else if (sum > 0) {
          --right;
        } else {
          ans.push_back({nums[i], nums[left], nums[right]});
          ++left;
          --right;
          while (left < right && nums[left] == nums[left - 1]) {
            ++left;
          }
          while (left < right && nums[right] == nums[right + 1]) {
            --right;
          }
        }
      }
    }
    return ans;
  }
};

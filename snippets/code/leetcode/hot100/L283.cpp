#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 public:
  void moveZeroes(vector<int>& nums) {
    int write = 0;
    for (int scan = 0; scan < static_cast<int>(nums.size()); ++scan) {
      if (nums[scan] != 0) {
        swap(nums[write], nums[scan]);
        ++write;
      }
    }
  }
};

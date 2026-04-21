#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 public:
  int jump(vector<int>& nums) {
    int n = nums.size();
    int steps = 0;
    int cur_end = 0;
    int farthest = 0;
    for (int i = 0; i < n - 1; ++i) {
      farthest = max(farthest, i + nums[i]);
      if (i == cur_end) {
        ++steps;
        cur_end = farthest;
      }
    }
    return steps;
  }
};

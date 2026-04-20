#include <unordered_map>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<int> twoSum(vector<int> &nums, int target) {
    unordered_map<int, int> pos;
    for (int i = 0; i < nums.size(); ++i) {
      int need = target - nums[i];
      // 如果补数已经出现过，直接得到答案。
      if (pos.count(need)) {
        return {pos[need], i};
      }
      // 记录当前值最早可用的位置。
      pos[nums[i]] = i;
    }
    return {};
  }
};

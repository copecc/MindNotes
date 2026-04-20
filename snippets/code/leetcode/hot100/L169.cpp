#include <vector>
using namespace std;

class Solution {
 public:
  int majorityElement(vector<int> &nums) {
    int candidate = nums[0];
    int count = 1;
    for (int i = 1; i < nums.size(); i++) {
      if (count == 0) {
        // 前面的票数已经完全抵消，从当前位置重新选择候选值。
        candidate = nums[i];
      }
      count += nums[i] == candidate ? 1 : -1;
    }
    return candidate;
  }
};

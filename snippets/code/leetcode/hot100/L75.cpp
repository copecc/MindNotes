#include <vector>
using namespace std;

class Solution {
 public:
  void sortColors(vector<int> &nums) {
    int left = 0;
    int current = 0;
    int right = nums.size() - 1;
    while (current <= right) {
      if (nums[current] == 0) {
        swap(nums[left], nums[current]);
        left++;
        current++;
      } else if (nums[current] == 2) {
        swap(nums[current], nums[right]);
        // 右侧换回来的元素还未检查，current 不能前进。
        right--;
      } else {
        current++;
      }
    }
  }
};

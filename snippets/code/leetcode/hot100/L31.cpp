#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 public:
  void nextPermutation(vector<int> &nums) {
    int n = nums.size();
    int i = n - 1;
    for (; i >= 1; --i) {
      if (nums[i] > nums[i - 1]) {
        break;
      }
    }
    // 整体非递增，当前已经是字典序最大排列。
    if (i == 0) {
      reverse(next(nums.begin(), i), nums.end());
      return;
    }
    int j = i;
    for (; j < n; ++j) {
      // 后缀非递增，回退一位后就是最后一个大于支点的元素。
      if (nums[j] > nums[i - 1]) {
        continue;
      }
      break;
    }
    j -= 1;
    swap(nums[i - 1], nums[j]);
    // 交换后将后缀变成最小升序排列。
    reverse(next(nums.begin(), i), nums.end());
  }
};

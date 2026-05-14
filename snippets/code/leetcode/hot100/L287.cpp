#include <vector>
using namespace std;

class Solution {
 public:
  int findDuplicate(vector<int> &nums) {
    int slow = 0;
    int fast = 0;
    do {
      // 把数组下标和取值看成 next 指针，重复数对应环入口。
      slow = nums[slow];
      fast = nums[nums[fast]];
    } while (slow != fast);

    int finder = 0;
    while (finder != slow) {
      finder = nums[finder];
      slow = nums[slow];
    }
    return finder;
  }
};


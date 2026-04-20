#include <vector>
using namespace std;

class Solution {
 public:
  int findDuplicate(vector<int> &nums) {
    int slow = 0;
    int fast = 0;
    do {
      // 把数组视为 next 指针，先在环内找到相遇点。
      slow = nums[slow];
      fast = nums[nums[fast]];
    } while (slow != fast);

    int finder = 0;
    while (finder != slow) {
      // 从起点和相遇点同步前进，再次相遇处就是环入口。
      finder = nums[finder];
      slow = nums[slow];
    }
    return finder;
  }
};

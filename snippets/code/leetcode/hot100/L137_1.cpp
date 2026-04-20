#include <vector>
using namespace std;

class Solution {
 public:
  int singleNumber(vector<int> &nums) {
    int ans = 0;
    for (int i = 0; i < 32; ++i) {
      int total = 0;
      for (int num : nums) {
        // 统计第 i 位上 1 的出现次数。
        total += (num >> i) & 1;
      }
      // 对 3 取模后剩下的就是目标值在这一位上的取值。
      if (total % 3 != 0) {
        ans |= 1 << i;
      }
    }
    return ans;
  }
};

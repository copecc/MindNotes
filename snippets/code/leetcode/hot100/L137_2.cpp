#include <vector>
using namespace std;

class Solution {
 public:
  int singleNumber(vector<int> &nums) {
    int ones = 0, twos = 0;
    for (int num : nums) {
      // ones 记录当前各二进制位出现 1 次后的状态。
      ones = (ones ^ num) & ~twos;
      // twos 记录当前各二进制位出现 2 次后的状态。
      twos = (twos ^ num) & ~ones;
    }
    // 出现 3 次的位会被自动清空，最终 ones 中剩下目标值。
    return ones;
  }
};

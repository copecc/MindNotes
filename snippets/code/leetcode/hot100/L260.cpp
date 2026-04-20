#include <vector>
using namespace std;

class Solution {
 public:
  vector<int> singleNumber(vector<int> &nums) {
    int xorResult = 0;
    for (int num : nums) {
      // 成对元素会抵消，最后得到两个目标值的异或结果。
      xorResult ^= num;
    }

    int mask = 1;
    while ((xorResult & mask) == 0) {
      // 找到一个可以区分这两个目标值的二进制位。
      mask <<= 1;
    }

    int num1 = 0, num2 = 0;
    for (int num : nums) {
      if (num & mask) {
        num1 ^= num;
      } else {
        num2 ^= num;
      }
    }
    return {num1, num2};
  }
};

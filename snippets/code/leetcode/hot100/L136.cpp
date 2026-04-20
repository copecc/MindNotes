#include <vector>
using namespace std;

class Solution {
 public:
  int singleNumber(vector<int> &nums) {
    int ans = 0;
    for (int x : nums) {
      // 成对元素异或后会抵消，最后剩下只出现一次的数。
      ans ^= x;
    }
    return ans;
  }
};

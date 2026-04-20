#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<int> singleNumberThree(vector<int> &nums) {
    auto lowbit = [](int x) -> unsigned int {
      unsigned int ux = static_cast<unsigned int>(x);
      return ux & (~ux + 1);
    };

    int xors = 0;
    for (int num : nums) {
      // 得到三个目标值的异或结果 a ^ b ^ c。
      xors ^= num;
    }

    unsigned int signature = 0;
    for (int num : nums) {
      // 成对元素的 lowbit(num ^ xors) 相同，异或后会抵消。
      signature ^= lowbit(num ^ xors);
    }

    // 取出一个能够命中其中某个目标值的特征位。
    unsigned int select = signature & (~signature + 1);
    int first = 0;
    for (int num : nums) {
      if (lowbit(num ^ xors) & select) {
        first ^= num;
      }
    }

    // 剩下两个数退化为“两个只出现一次的数字”。
    int rest = xors ^ first;
    unsigned int split = lowbit(rest);
    int second = 0, third = 0;
    for (int num : nums) {
      // 已经恢复出 first，再次异或它没有意义。
      if (num == first) {
        continue;
      }
      if (static_cast<unsigned int>(num) & split) {
        second ^= num;
      } else {
        third ^= num;
      }
    }

    vector<int> ans = {first, second, third};
    sort(ans.begin(), ans.end());
    return ans;
  }
};

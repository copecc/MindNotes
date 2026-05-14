#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 public:
  int lengthOfLIS(vector<int>& nums) {
    vector<int> tails;
    for (int num : nums) {
      auto it = lower_bound(tails.begin(), tails.end(), num);
      if (it == tails.end()) {
        tails.push_back(num);
      } else {
        *it = num;  // 把同长度子序列的结尾压到更小，为后续数字留空间。
      }
    }
    return tails.size();
  }
};

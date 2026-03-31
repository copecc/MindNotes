#include <unordered_map>
#include <vector>
using namespace std;

class Solution {
 public:
  int findMaxLength(vector<int> &nums) {
    unordered_map<int, int> first;
    first[0]   = -1;  // 前缀和为0时，起点在 -1
    int prefix = 0, res = 0;
    for (int i = 0; i < nums.size(); ++i) {
      prefix += (nums[i] == 1 ? 1 : -1);
      if (first.count(prefix)) {
        res = max(res, i - first[prefix]);
      } else {
        first[prefix] = i;
      }
    }
    return res;
  }
};
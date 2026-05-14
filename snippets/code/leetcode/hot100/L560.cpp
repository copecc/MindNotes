#include <unordered_map>
#include <vector>
using namespace std;

class Solution {
 public:
  int subarraySum(vector<int>& nums, int k) {
    unordered_map<int, int> cnt;
    cnt[0] = 1;

    int sum = 0, ans = 0;
    for (int x : nums) {
      sum += x;
      auto it = cnt.find(sum - k);
      if (it != cnt.end()) {
        ans += it->second;  // 先统计以当前位置结尾的答案，再把当前前缀和加入计数。
      }
      ++cnt[sum];
    }
    return ans;
  }
};


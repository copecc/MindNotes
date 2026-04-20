#include <algorithm>
#include <unordered_set>
#include <vector>
using namespace std;

class Solution {
 public:
  int longestConsecutive(vector<int> &nums) {
    unordered_set<int> s(nums.begin(), nums.end());
    int ans = 0;

    for (int x : s) {
      // 只有当 x 是连续段起点时才向后扩展。
      if (s.count(x - 1)) {
        continue;
      }

      int y = x;
      while (s.count(y + 1)) {
        y++;
      }
      ans = max(ans, y - x + 1);
    }
    return ans;
  }
};

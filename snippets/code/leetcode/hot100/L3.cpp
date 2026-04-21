#include <algorithm>
#include <unordered_map>
#include <string>
using namespace std;

class Solution {
 public:
  int lengthOfLongestSubstring(string s) {
    unordered_map<char, int> last;
    int left = 0, ans = 0;
    for (int right = 0; right < static_cast<int>(s.size()); ++right) {
      auto it = last.find(s[right]);
      if (it != last.end()) {
        left = max(left, it->second + 1);
      }
      last[s[right]] = right;
      ans = max(ans, right - left + 1);
    }
    return ans;
  }
};

#include <string>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<int> findAnagrams(string s, string p) {
    if (s.size() < p.size()) {
      return {};
    }

    vector<int> need(26);
    for (char c : p) {
      ++need[c - 'a'];
    }

    vector<int> ans;
    int missing = static_cast<int>(p.size());
    int left = 0;
    for (int right = 0; right < static_cast<int>(s.size()); ++right) {
      if (need[s[right] - 'a']-- > 0) {
        --missing;
      }
      if (right - left + 1 > static_cast<int>(p.size())) {
        if (++need[s[left] - 'a'] > 0) {
          ++missing;
        }
        ++left;
      }
      if (missing == 0) {
        ans.push_back(left);
      }
    }
    return ans;
  }
};

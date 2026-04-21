#include <climits>
#include <string>
#include <vector>
using namespace std;

class Solution {
 public:
  string minWindow(string s, string t) {
    if (s.size() < t.size()) {
      return "";
    }

    vector<int> need(128);
    for (char c : t) {
      ++need[c];
    }

    int missing = static_cast<int>(t.size());
    int bestLeft = 0, bestLen = INT_MAX;
    int left = 0;
    for (int right = 0; right < static_cast<int>(s.size()); ++right) {
      if (need[s[right]]-- > 0) {
        --missing;
      }

      while (missing == 0) {
        if (right - left + 1 < bestLen) {
          bestLen = right - left + 1;
          bestLeft = left;
        }
        if (++need[s[left]] > 0) {
          ++missing;
        }
        ++left;
      }
    }

    return bestLen == INT_MAX ? "" : s.substr(bestLeft, bestLen);
  }
};

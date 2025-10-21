#include <string>
#include <unordered_map>
using namespace std;

class Solution {
 public:
  int findTheLongestSubstring(string s) {
    unordered_map<int, int> first;
    first[0] = -1;
    int mask = 0, res = 0;

    for (int i = 0; i < s.size(); ++i) {
      if (string("aeiou").find(s[i]) != string::npos) { mask ^= 1 << (s[i] - 'a'); }
      if (first.count(mask)) {
        res = max(res, i - first[mask]);
      } else {
        first[mask] = i;
      }
    }
    return res;
  }
};
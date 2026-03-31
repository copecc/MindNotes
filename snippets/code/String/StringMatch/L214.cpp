#include <vector>
using namespace std;

class Solution {
  vector<int> get_next(const string &pattern) {
    vector<int> next(pattern.length());
    // j 表示当前匹配的前缀长度
    for (int i = 1, j = 0; i < pattern.length(); ++i) {
      while (j > 0 && pattern[i] != pattern[j]) { j = next[j - 1]; }
      if (pattern[i] == pattern[j]) { ++j; }
      next[i] = j;
    }
    return next;
  }

 public:
  string shortestPalindrome(string s) {
    string rev_s = s;
    reverse(rev_s.begin(), rev_s.end());
    string combined  = s + '#' + rev_s;  // '#' 是分隔符, 不会出现在 s 中
    vector<int> next = get_next(combined);
    int len          = next.back();  // 最长回文前缀的长度
    return rev_s.substr(0, rev_s.length() - len) + s;
  }
};
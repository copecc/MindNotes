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
  int strStr(string text, string pattern) {
    if (pattern.empty()) { return 0; }
    vector<int> next = get_next(pattern);
    // i 表示 text 的匹配位置, j 表示 pattern 的匹配位置
    for (int i = 0, j = 0; i < text.length(); i++) {
      while (j > 0 && pattern[j] != text[i]) {  // 没有匹配则回退, 重新尝试匹配
        j = next[j - 1];  // 回退到下一个可能匹配的位置, 此时 [0,j-1] 已经匹配成功
      }
      // 匹配成功, 尝试匹配下一个字符
      if (text[i] == pattern[j]) { ++j; }
      // 匹配整个模式串
      if (j == pattern.length()) {
        return i - j + 1;  // 返回第一个匹配位置
      }
    }
    return -1;  // 如果没有匹配, 返回 -1
  }
};
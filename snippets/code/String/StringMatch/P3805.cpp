#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

pair<int, int> manacher(const string &s) {
  int n = 2 * s.length() + 1;
  string t(n, 0);
  for (int i = 0, j = 0; i < n; ++i) { t[i] = (i & 1) == 0 ? '#' : s[j++]; }
  vector<int> p(n, 0);        // p[i] 表示以 t[i] 为中心的最长回文子串的半径(不含中心)
  int center = 0, right = 0;  // 当前回文子串的中心和右边界
  for (int i = 0; i < n; ++i) {
    int len = right > i ? min(p[2 * center - i], right - i) : 1;
    // 尝试扩展回文子串
    while (i - len >= 0 && i + len < n && t[i - len] == t[i + len]) { ++len; }
    p[i] = len;
    // 更新回文子串的中心和右边界
    if (i + len > right) {
      center = i;
      right  = i + len;
    }
  }
  // 找到最长回文子串
  int max_len = 0, start = 0;
  for (int i = 0; i < n; ++i) {
    if (p[i] > max_len) {
      max_len = p[i];
      start   = (i - max_len + 1) / 2;  // 真实回文串的起始位置为 (i - p[i] + 1) / 2
    }
  }
  return {start, max_len - 1};  // 返回最长回文子串的起始位置和长度
}

int main() {
  string s;
  cin >> s;
  auto [start, len] = manacher(s);
  cout << len << '\n';
  return 0;
}
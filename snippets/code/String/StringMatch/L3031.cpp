#include <vector>
using namespace std;

class Solution {
  vector<int> get_z(const string &s) {
    int n = s.length();
    vector<int> z(n, 0);
    z[0] = n;
    // [l, r) 表示当前匹配区间，区间内 s[l...r-1] 与 s[0...(r-l-1)] 匹配
    for (int i = 1, l = 0, r = 0; i < n; ++i) {
      int len = r > i ? min(z[i - l], r - i) : 0;
      // 尝试扩展匹配区间
      while (i + len < n && s[len] == s[i + len]) { ++len; }
      z[i] = len;
      // 如果匹配区间扩展到 r 右边，更新 [l,r)
      if (i + len > r) {
        l = i;
        r = i + len;
      }
    }
    return z;
  }

 public:
  int minimumTimeToInitialState(string word, int k) {
    int n         = word.length();
    vector<int> z = get_z(word);
    for (int i = k; i < n; i += k) {
      if (z[i] + i == n) { return i / k; }
    }
    return (n + k - 1) / k;  // 无法匹配, 只能每次移除 k 个字符后添加回来
  }
};
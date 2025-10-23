#include <cstdint>
#include <string>
#include <unordered_map>
using namespace std;

class Solution {
 public:
  int longestBalanced(string s) {
    int n   = s.size();
    int res = 0;
    {  // 一种字母相同
      for (int i = 0; i < n;) {
        int start = i;
        for (i++; i < n && s[i] == s[i - 1]; i++);
        res = max(res, i - start);
      }
    }
    {  // 两种字母相同
      auto count = [&](char x, char y) -> void {
        for (int i = 0; i < n; i++) {  // 枚举起点, 跳过不包含 x 和 y 的字符
          unordered_map<int, int> first;
          first[0]   = i - 1;  // 前缀和为0时，起点在 i - 1
          int prefix = 0;      // x 的个数减去 y 的个数
          for (; i < n && (s[i] == x || s[i] == y); i++) {
            prefix += s[i] == x ? 1 : -1;
            if (first.contains(prefix)) {
              res = max(res, i - first[prefix]);
            } else {
              first[prefix] = i;
            }
          }
        }
      };
      count('a', 'b');
      count('a', 'c');
      count('b', 'c');
    }
    {  // 三种字母相同
      unordered_map<int64_t, int> first;
      int64_t cnt0 = 0, cnt1 = 0, cnt2 = 0;

      auto encode
          = [&](int64_t x, int64_t y) -> int64_t { return (x << 32) ^ (y & 0xFF'FF'FF'FF); };

      first[encode(0, 0)] = -1;  // 初始状态

      for (int i = 0; i < s.size(); ++i) {
        if (s[i] == 'a') {
          cnt0++;
        } else if (s[i] == 'b') {
          cnt1++;
        } else {
          cnt2++;
        }
        int64_t key = encode(cnt1 - cnt0, cnt2 - cnt0);
        if (first.count(key)) {
          res = max(res, i - first[key]);
        } else {
          first[key] = i;
        }
      }

      return res;
    }
  }
};
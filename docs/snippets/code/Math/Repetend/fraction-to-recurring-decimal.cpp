#include <string>
#include <unordered_map>
using namespace std;

class Solution {
 public:
  string fractionToDecimal(int numerator, int denominator) {
    if (numerator == 0) { return "0"; }
    string res;
    // 判断符号
    if ((numerator < 0) ^ (denominator < 0)) { res += '-'; }
    // 转为正数，防止溢出
    int64_t n = abs(static_cast<int64_t>(numerator));
    int64_t d = abs(static_cast<int64_t>(denominator));
    // 整数部分
    res       += to_string(n / d);
    int64_t r  = n % d;
    if (r == 0) { return res; }  // 无小数部分
    res += '.';
    unordered_map<int64_t, int> mp;  // 记录余数及其对应的位置
    while (r != 0) {
      if (mp.count(r) != 0U) {  // 出现循环
        res.insert(mp[r], "(");
        res += ')';
        break;
      }
      mp[r]  = res.size();
      r     *= 10;
      res   += to_string(r / d);
      r     %= d;
    }
    return res;
  }
};
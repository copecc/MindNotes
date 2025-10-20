#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

using VVVI = vector<vector<vector<int64_t>>>;
VVVI memo;  // 记忆化数组

int64_t digit_dp(string &s) {
  int64_t m = s.length();
  s.push_back('0');
  reverse(s.begin(), s.end());

  vector<int64_t> fill(m + 1, 0);
  // start表示非0的起始位置，那么从这里开始的长度是m-start，最多只用遍历一半
  using dfs_type = function<int64_t(int64_t, bool, bool, int64_t)>;
  dfs_type dfs   = [&](int64_t i, bool is_limit, bool ok, int64_t start) -> int64_t {
    if (i == 0) { return ok; }  // 枚举完所有数位, ok表示是否对称
    if (!is_limit && memo[i][start][ok] != -1) { return memo[i][start][ok]; }
    int64_t res = 0;

    int64_t up  = is_limit ? s[i] - '0' : 9;
    // 枚举要填入的数字 d
    for (int64_t d = 0; d <= up; ++d) {
      if (d != 0 && d != 1 && d != 8) {  // 不能有非0,1,8
        continue;
      }
      fill[i]            = d;
      int64_t new_ok     = (ok && i <= start / 2) ? d == fill[start - i + 1] : ok;  // 对称性
      int64_t new_start  = (start == i && d == 0) ? start - 1 : start;  // 更新非0起始位置
      res               += dfs(i - 1, is_limit && d == up, new_ok, new_start);
    }

    if (!is_limit) { memo[i][start][ok] = res; }
    return res;
  };
  return dfs(m, true, true, m);
}

int64_t check(const string &s) {
  int64_t m   = s.length();
  int64_t res = 0;
  for (int64_t i = 0; i < m; i++) {
    if (s[i] != '0' && s[i] != '1' && s[i] != '8') {  // 不能有非0,1,8
      return 0;
    }
    if (s[i] != s[m - i - 1]) {  // 对称
      return 0;
    }
  }
  return 1;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t t;
  cin >> t;
  size_t m = 0;
  vector<string> low(t), high(t);
  for (int64_t i = 0; i < t; i++) {
    cin >> low[i] >> high[i];
    m = max({m, low[i].length(), high[i].length()});
  }
  // 多个测试用例时，复用memo数组, 避免memo重新计算
  memo.resize(m + 1, vector<vector<int64_t>>(m + 1, vector<int64_t>(2, -1)));
  for (int i = 0; i < t; i++) {
    int is_low       = check(low[i]);  // low本身是否符合要求
    int64_t high_ans = digit_dp(high[i]);
    int64_t low_ans  = digit_dp(low[i]);
    cout << high_ans - low_ans + is_low << "\n";
  }
}
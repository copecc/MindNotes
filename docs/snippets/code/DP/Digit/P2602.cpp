#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int64_t digit_dp(const string &s, int num) {
  int m = s.length();
  vector<vector<int64_t>> memo(m, vector<int64_t>(m, -1));
  // count: 当前已经填了多少个 num
  using dfs_type = function<int64_t(int64_t, bool, bool, int)>;
  dfs_type dfs   = [&](int i, bool is_limit, bool is_num, int count) -> int64_t {
    if (i == m) { return is_num ? count : 0; }
    if (!is_limit && is_num && memo[i][count] != -1) { return memo[i][count]; }
    int64_t res = 0;
    // 可以跳过当前数位
    if (!is_num) { res = dfs(i + 1, false, false, count); }
    int up = is_limit ? s[i] - '0' : 9;
    // 枚举要填入的数字 d
    for (int d = 1 - is_num; d <= up; ++d) {
      res = res + dfs(i + 1, is_limit && d == up, true, count + (d == num));
    }
    // 记忆化结果
    if (!is_limit && is_num) { memo[i][count] = res; }
    return res;
  };
  return dfs(0, true, false, 0);
}

int main() {
  string low;
  string high;
  cin >> low >> high;
  vector<int64_t> count(10);
  for (char ch : low) { count[ch - '0']++; }
  for (int i = 0; i <= 9; ++i) { cout << digit_dp(high, i) - digit_dp(low, i) + count[i] << " "; }
}
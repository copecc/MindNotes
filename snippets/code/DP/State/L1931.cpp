#include <cstdint>
#include <functional>
#include <utility>
#include <vector>
using namespace std;

class Solution {
 public:
  int colorTheGrid(int m, int n) {
    const int mod  = 1e9 + 7;

    auto get_state = [](int64_t s, int64_t pow_j) { return (s / pow_j) % 3; };
    auto set_state = [](int64_t s, int64_t pow_j, int v) {
      int64_t old = (s / pow_j) % 3;
      return s + (v - old) * pow_j;
    };

    if (m > n) { swap(m, n); }  // 保证 m <= n
    int64_t max_state = pow(3, m);
    // 获取一行有效状态
    vector<int> initial_states;
    std::function<void(int, int, int)> dfs_init = [&](int j, int s, int pow_j) {
      if (j == m) {
        initial_states.push_back(s);
        return;
      }
      // 获取左边的状态, pow_j / 3 就是 3^(j-1)
      int left = (j == 0) ? -1 : get_state(s, pow_j / 3);
      for (int v = 0; v < 3; ++v) {
        if (v != left) { dfs_init(j + 1, set_state(s, pow_j, v), pow_j * 3); }
      }
    };
    dfs_init(0, 0, 1);
    // 分别对应行、列、状态空间
    auto dp = vector(n, vector(m, vector<int64_t>(pow(3, m), -1)));
    function<int64_t(int, int, int64_t, int64_t)> dfs
        = [&](int i, int j, int64_t s, int64_t pow_j) -> int64_t {
      // 所有行处理完
      if (i == n) { return 1; }
      if (j == m) { return dfs(i + 1, 0, s, 1); }  // 换行
      if (dp[i][j][s] != -1) { return dp[i][j][s]; }
      int64_t res = 0;
      // 获取当前位置的左边和上边的状态
      int left = (j == 0) ? -1 : get_state(s, pow_j / 3);
      int up   = get_state(s, pow_j);
      for (int v = 0; v < 3; ++v) {
        if (v != left && v != up) {  // 与左边和上边不同色
          res = (res + dfs(i, j + 1, set_state(s, pow_j, v), pow_j * 3)) % mod;
        }
      }
      dp[i][j][s] = res;
      return res;
    };
    int64_t ans = 0;
    for (int s : initial_states) { ans = (ans + dfs(1, 0, s, 1)) % mod; }
    return ans;
  }
};
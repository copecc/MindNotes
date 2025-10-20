#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int N, K;
  cin >> N >> K;

  auto dp      = vector(N, vector(N, vector(1 << N, vector(2, vector<int64_t>(K + 1, -1)))));
  auto get_bit = [](int state, int j) { return (state >> j) & 1; };
  auto set_bit = [](int state, int j, int value) {
    return value == 0 ? (state & ~(1 << j)) : (state | (1 << j));
  };
  // i: 当前行, j: 当前列, state: 当前行状态, left_up: 左上角是否放置, rest_k:
  // 剩余国王数量
  std::function<int64_t(int, int, int, int, int)> dfs
      = [&](int i, int j, int state, int left_up, int rest_k) -> int64_t {
    // 所有行处理完, 且正好放置k个国王
    if (i == N) { return rest_k == 0; }
    if (j == N) { return dfs(i + 1, 0, state, 0, rest_k); }  // 换行
    if (dp[i][j][state][left_up][rest_k] != -1) { return dp[i][j][state][left_up][rest_k]; }
    // 获取当前位置的左边, 上边, 右上角的状态, 左上角状态由参数获得
    int left     = (j == 0) ? 0 : get_bit(state, j - 1);
    int up       = get_bit(state, j);
    int right_up = (j == N - 1) ? 0 : get_bit(state, j + 1);
    int64_t res  = 0;
    // 从下一列继续, 设置当前位置为0, 下一个位置的左上角状态为当前位置的上方状态
    // 不放置国王
    res += dfs(i, j + 1, set_bit(state, j, 0), up, rest_k);
    // 放置国王, 当前位置的左边, 左上角, 上边, 右上角都不能有国王,
    // 且剩余国王数量要大于0
    if (rest_k > 0 && left == 0 && left_up == 0 && up == 0 && right_up == 0) {
      res += dfs(i, j + 1, set_bit(state, j, 1), up, rest_k - 1);
    }
    dp[i][j][state][left_up][rest_k] = res;  // 记忆化
    return res;
  };
  cout << dfs(0, 0, 0, 0, K) << "\n";
  return 0;
}
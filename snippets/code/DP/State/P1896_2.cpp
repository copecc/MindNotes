#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int N, K;
  cin >> N >> K;

  auto get_bit = [](int state, int j) { return (state >> j) & 1; };
  auto set_bit = [](int state, int j, int value) {
    return value == 0 ? (state & ~(1 << j)) : (state | (1 << j));
  };
  auto dp = vector(N + 1, vector(1 << N, vector(2, vector<int64_t>(K + 1))));
  for (int s = 0; s < (1 << N); ++s) {
    for (int k = 0; k <= K; k++) { dp[0][s][0][k] = k == 0 ? 1 : 0; }
  }
  for (int i = N - 1; i >= 0; --i) {
    // j == N
    for (int s = 0; s < (1 << N); ++s) {
      for (int k = 0; k <= K; k++) { dp[N][s][0][k] = dp[N][s][1][k] = dp[0][s][0][k]; }
    }
    // j < N
    for (int j = N - 1; j >= 0; --j) {
      for (int s = 0; s < (1 << N); ++s) {
        for (int left_up = 0; left_up <= 1; ++left_up) {
          for (int k = 0; k <= K; k++) {
            // 获取当前位置的左边, 上边, 右上角的状态, 左上角状态由参数获得
            int left     = (j == 0) ? 0 : get_bit(s, j - 1);
            int up       = get_bit(s, j);
            int right_up = (j == N - 1) ? 0 : get_bit(s, j + 1);
            int64_t res  = 0;
            // 从下一列继续, 设置当前位置为0,
            // 下一个位置的左上角状态为当前位置的上方状态 不放置国王
            res += dp[j + 1][set_bit(s, j, 0)][up][k];
            // 放置国王, 当前位置的左边, 左上角, 上边, 右上角都不能有国王,
            // 且剩余国王数量要大于0
            if (k > 0 && left == 0 && left_up == 0 && up == 0 && right_up == 0) {
              res += dp[j + 1][set_bit(s, j, 1)][up][k - 1];
            }
            dp[j][s][left_up][k] = res;
          }
        }
      }
    }
  }
  cout << dp[0][0][0][K] << "\n";
  return 0;
}
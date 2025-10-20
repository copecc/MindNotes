#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n, W;
  cin >> n >> W;
  vector<int> weights(n + 1), values(n + 1), counts(n + 1);
  for (int i = 1; i <= n; i++) { cin >> values[i] >> weights[i] >> counts[i]; }
  vector<int> dp(W + 1, 0);
  for (int i = 1; i <= n; i++) {
    int w = weights[i], v = values[i], c = counts[i];
    for (int m = 0; m < w; m++) {  // 按余数分类
      deque<pair<int, int>> dq;    // {t, g(t)} (1)
      for (int t = 0; m + t * w <= W; t++) {
        int j = m + t * w;
        // 队列头元素过期
        while (!dq.empty() && t - dq.front().first > c) { dq.pop_front(); }
        int g = dp[j] - t * v;
        // 单调队列维护最大值, 根据指标 g(t) = dp[i-1][j] - t*v 进行维护
        while (!dq.empty() && dq.back().second <= g) { dq.pop_back(); }
        dq.emplace_back(t, g);
        dp[j] = dq.front().second + t * v;
      }
    }
  }

  cout << dp[W] << "\n";
  return 0;
}
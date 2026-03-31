#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n;
  cin >> n;
  vector<int64_t> nums(n);
  for (int i = 0; i < n; ++i) { cin >> nums[i]; }
  // 最大平均值问题
  {
    auto check = [&](double x) {
      vector<double> diffs(n);
      for (int i = 0; i < n; ++i) { diffs[i] = nums[i] - x; }
      // 判定条件, i 或 i + 1 必须至少选择其中一个
      vector<vector<double>> dp(n, vector<double>(2, 0));
      dp[0][0] = 0;         // i 不选
      dp[0][1] = diffs[0];  // i 选
      for (int i = 1; i < n; ++i) {
        dp[i][0] = dp[i - 1][1];                                // i 不选, 则 i-1 必须选
        dp[i][1] = max(dp[i - 1][0], dp[i - 1][1]) + diffs[i];  // i 选, 则 i-1 可选可不选
      }
      return max(dp[n - 1][0], dp[n - 1][1]) >= 0;
    };

    double left = 1, right = 1e9 + 1;
    for (int i = 0; i < 64; ++i) {  // 进行足够次数的二分
      double mid = (left + right) / 2;
      if (check(mid)) {
        left = mid;
      } else {
        right = mid;
      }
    }
    cout << left << "\n";
  }
  // 最大上中位数问题
  {
    auto check = [&](int64_t x) {
      vector<int64_t> diffs(n);
      for (int i = 0; i < n; ++i) { diffs[i] = nums[i] >= x ? 1 : -1; }
      // 判定条件, i 或 i + 1 必须至少选择其中一个
      vector<vector<int64_t>> dp(n, vector<int64_t>(2, 0));
      dp[0][0] = 0;         // i 不选
      dp[0][1] = diffs[0];  // i 选
      for (int i = 1; i < n; ++i) {
        dp[i][0] = dp[i - 1][1];                                // i 不选, 则 i-1 必须选
        dp[i][1] = max(dp[i - 1][0], dp[i - 1][1]) + diffs[i];  // i 选, 则 i-1 可选可不选
      }
      return max(dp[n - 1][0], dp[n - 1][1]) > 0;  // 注意这里是大于0
    };

    int64_t left = 1, right = 1e9 + 1;
    for (int i = 0; i < 64; ++i) {  // 进行足够次数的二分
      int64_t mid = (left + right) / 2;
      if (check(mid)) {
        left = mid;
      } else {
        right = mid;
      }
    }
    cout << left << "\n";
  }
  return 0;
}
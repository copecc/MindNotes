#include <iostream>
#include <vector>
using namespace std;

int64_t derangement(int64_t n) {
  if (n == 0) { return 1; }
  if (n == 1) { return 0; }
  vector<int64_t> dp(n + 1, 0);
  dp[0] = 1;
  dp[1] = 0;
  for (int64_t i = 2; i <= n; ++i) { dp[i] = (i - 1) * (dp[i - 1] + dp[i - 2]); }
  return dp[n];
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n;
  cin >> n;
  cout << derangement(n) << "\n";
  return 0;
}
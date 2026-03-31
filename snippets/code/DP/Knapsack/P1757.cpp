#include <algorithm>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

int main() {
  int m, n;
  cin >> m >> n;
  vector<int> weights(n + 1), values(n + 1);
  map<int, vector<int>> groups;
  for (int i = 1; i <= n; ++i) {
    int w, v, g;
    cin >> w >> v >> g;
    groups[g].push_back(i);
    weights[i] = w;
    values[i]  = v;
  }
  vector<int> dp(m + 1, 0);
  for (const auto &[_, items] : groups) {
    for (int j = m; j >= 0; --j) {
      for (auto i : items) {
        if (j >= weights[i]) { dp[j] = max(dp[j], dp[j - weights[i]] + values[i]); }
      }
    }
  }
  cout << dp[m] << '\n';
}
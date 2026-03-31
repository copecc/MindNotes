#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

// 矩阵乘法，定义为 "最小化最大值"
vector<vector<int64_t>> multiply(const vector<vector<int64_t>> &a,
                                 const vector<vector<int64_t>> &b) {
  int n = a.size();
  vector<vector<int64_t>> c(n, vector<int64_t>(n, INT64_MAX));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      for (int k = 0; k < n; ++k) { c[i][j] = min(c[i][j], max(a[i][k], b[k][j])); }
    }
  }
  return c;
}

// 矩阵快速幂
vector<vector<int64_t>> pow(vector<vector<int64_t>> base, int64_t exp) {
  int n = base.size();
  // 零元矩阵: min(a, INT64_MAX) = a
  vector<vector<int64_t>> result(n, vector<int64_t>(n, INT64_MAX));
  for (int i = 0; i < n; ++i) {
    result[i][i] = INT64_MIN;  // 单位元: max(a, INT64_MIN) = a
  }
  while (exp > 0) {
    if ((exp & 1) != 0) { result = multiply(result, base); }
    base   = multiply(base, base);
    exp  >>= 1;
  }
  return result;
}

int main() {
  int n, t, l;
  cin >> n >> t >> l;
  vector<vector<int64_t>> g(n, vector<int64_t>(n, INT64_MAX));
  for (int i = 1; i <= t; ++i) {
    int u, v;
    cin >> u >> v;
    --u, --v;
    g[u][v] = i;
  }
  // dp[i][j]: 从 0 走到 j, 恰好走 i 步的路径中, 最大边权的最小值
  // dp[i][j] = min{ max(dp[i-1][k], g[k][j]) | k = 0..n-1 }
  // 矩阵乘法定义为: c[i][j] = min{ max(a[i][k], b[k][j]) | k = 0..n-1 }
  // dp[i] = dp[i-1] * g
  // dp[l] = dp[0] * g^l = I * g^l = g^l
  // 所以只需要计算 g^l 即可
  vector<vector<int64_t>> result = pow(g, l);
  for (int i = 0; i < n; ++i) {
    if (result[0][i] == INT64_MAX) {
      cout << "-1 ";
    } else {
      cout << result[0][i] << " ";
    }
  }
  cout << '\n';
  return 0;
}
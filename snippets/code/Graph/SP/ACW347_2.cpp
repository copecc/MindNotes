#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>
using namespace std;

// 矩阵乘法，定义为 "最小化加法"（最短路）
vector<vector<int>> multiply(const vector<vector<int>> &a, const vector<vector<int>> &b) {
  int n = a.size();
  vector<vector<int>> c(n, vector<int>(n, INT32_MAX / 2));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      for (int k = 0; k < n; ++k) { c[i][j] = min(c[i][j], a[i][k] + b[k][j]); }
    }
  }
  return c;
}

// 矩阵快速幂
vector<vector<int>> pow(vector<vector<int>> base, int exp) {
  int n = base.size();
  // 零元矩阵: min(a, INT32_MAX / 2) = a
  vector<vector<int>> result(n, vector<int>(n, INT32_MAX / 2));
  for (int i = 0; i < n; ++i) { result[i][i] = 0; }  // 单位元: a + 0 = a
  while (exp > 0) {
    if ((exp & 1) != 0) { result = multiply(result, base); }
    base   = multiply(base, base);
    exp  >>= 1;
  }
  return result;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  using TIII = tuple<int, int, int>;

  int n, t, s, e;
  cin >> n >> t >> s >> e;
  vector<TIII> edges(t);
  map<int, int> point_id;
  for (int i = 0; i < t; ++i) {
    int l, x, y;
    cin >> l >> x >> y;
    if (point_id.count(x) == 0) { point_id[x] = point_id.size(); }
    if (point_id.count(y) == 0) { point_id[y] = point_id.size(); }
    edges[i] = {l, point_id[x], point_id[y]};
  }
  s         = point_id[s];
  e         = point_id[e];
  int m     = point_id.size();

  using VI  = vector<int>;
  using VVI = vector<VI>;

  VVI graph(m, VI(m, INT32_MAX / 2));
  for (const auto &[l, x, y] : edges) { graph[x][y] = graph[y][x] = min(graph[x][y], l); }
  VVI result = pow(graph, n);
  cout << result[s][e] << "\n";
  return 0;
}
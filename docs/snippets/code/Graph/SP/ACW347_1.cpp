#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>
using namespace std;

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

  VVI dp(m, VI(m, INT32_MAX / 2));
  for (int j = 0; j < m; ++j) { dp[j][j] = 0; }
  VVI dp_next(m, VI(m, INT32_MAX / 2));
  for (int i = 1; i <= n; ++i) {  // 枚举步数
    fill(dp_next.begin(), dp_next.end(), VI(m, INT32_MAX / 2));
    for (int j = 0; j < m; ++j) {      // 枚举起点
      for (int k = 0; k < m; ++k) {    // 枚举终点
        for (int l = 0; l < m; ++l) {  // 枚举中转点
          dp_next[j][k] = min(dp_next[j][k], dp[j][l] + graph[l][k]);
        }
      }
    }
    dp = dp_next;
  }
  cout << dp[s][e] << "\n";
  return 0;
}
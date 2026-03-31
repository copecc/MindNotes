#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

void solve() {}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, m;
  cin >> n >> m;
  vector<vector<int64_t>> distance(n + 1, vector<int64_t>(n + 1, INT64_MAX));
  vector<int64_t> path(m);
  for (int64_t i = 0; i < m; ++i) { cin >> path[i]; }
  for (int64_t i = 1; i <= n; ++i) {
    for (int64_t j = 1; j <= n; ++j) { cin >> distance[i][j]; }
  }
  // floyd
  for (int k = 1; k <= n; ++k) {  // 枚举中转点
    for (int i = 1; i <= n; ++i) {
      for (int j = 1; j <= n; ++j) {
        // 找到更短路径
        if (distance[i][k] != INT64_MAX && distance[k][j] != INT64_MAX
            && distance[i][k] + distance[k][j] < distance[i][j]) {
          distance[i][j] = distance[i][k] + distance[k][j];
        }
      }
    }
  }
  int64_t ans = 0;
  for (int64_t i = 1; i < m; ++i) { ans += distance[path[i - 1]][path[i]]; }
  cout << ans << "\n";

  return 0;
}
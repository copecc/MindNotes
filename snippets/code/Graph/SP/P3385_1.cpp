#include <cstdint>
#include <iostream>
#include <tuple>
#include <vector>
using namespace std;

void solve() {
  const int64_t INF = INT64_MAX;

  int64_t n, m;
  cin >> n >> m;
  using TIII = tuple<int64_t, int64_t, int64_t>;  // (from, to, weight)
  vector<TIII> edges;
  for (int64_t i = 0; i < m; ++i) {
    int64_t u, v, w;
    cin >> u >> v >> w;
    if (w < 0) {
      edges.emplace_back(u, v, w);
    } else {
      edges.emplace_back(u, v, w);
      edges.emplace_back(v, u, w);
    }
  }
  int64_t source = 1;

  vector<int64_t> backup;
  vector<int64_t> distance(n + 1, INF);
  distance[source] = 0;
  // 最多松弛n-1次
  for (int i = 0; i < n - 1; ++i) {
    backup = distance;
    for (const auto &edge : edges) {
      auto [from, to, weight] = edge;
      // 松弛操作
      if (backup[from] != INF && distance[to] > backup[from] + weight) {
        distance[to] = backup[from] + weight;
      }
    }
  }
  // 检测负环
  for (const auto &edge : edges) {
    auto [from, to, weight] = edge;
    if (distance[from] != INF && distance[to] > distance[from] + weight) {  // 有负环
      cout << "YES\n";
      return;
    }
  }
  cout << "NO\n";
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t t;
  cin >> t;
  while ((t--) != 0) { solve(); }
}
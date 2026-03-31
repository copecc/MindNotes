#include <cstdint>
#include <functional>
#include <iostream>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;

using PII  = pair<int64_t, int64_t>;
using TIII = tuple<int64_t, int64_t, int64_t>;

int64_t dijkstra(vector<vector<PII>> &graph, int64_t source, int64_t end, int64_t k) {
  int64_t n = graph.size();
  vector<vector<int64_t>> distance(n, vector<int64_t>(k + 1, INT32_MAX));
  vector<vector<bool>> visit(n, vector<bool>(k + 1, false));
  distance[source][0] = 0;

  priority_queue<TIII, vector<TIII>, greater<>> pq;  // {distance, node, k_used}
  pq.emplace(0, source, 0);

  while (!pq.empty()) {
    auto [d, u, k_used] = pq.top();
    pq.pop();
    if (u == end) { return d; }
    if (visit[u][k_used]) { continue; }
    visit[u][k_used] = true;
    for (auto [v, w] : graph[u]) {
      if (distance[u][k_used] + w < distance[v][k_used]) {  // 不使用
        distance[v][k_used] = distance[u][k_used] + w;
        pq.emplace(distance[v][k_used], v, k_used);
      }
      if (k_used < k && distance[u][k_used] + 0 < distance[v][k_used + 1]) {  // 使用
        distance[v][k_used + 1] = distance[u][k_used];
        pq.emplace(distance[v][k_used + 1], v, k_used + 1);
      }
    }
  }
  return -1;
}

int main() {
  int64_t n, m, k;
  cin >> n >> m >> k;
  int64_t s, t;
  cin >> s >> t;
  vector<vector<PII>> graph(n);
  for (int64_t i = 0; i < m; ++i) {
    int64_t u, v, w;
    cin >> u >> v >> w;
    graph[u].emplace_back(v, w);
    graph[v].emplace_back(u, w);  // 无向图
  }
  cout << dijkstra(graph, s, t, k) << '\n';
  return 0;
}
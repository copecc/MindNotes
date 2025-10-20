#include <cstdint>
#include <functional>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

using PII = pair<int64_t, int64_t>;

void dijkstra(int n, const vector<vector<PII>> &graph, int64_t source) {
  vector<int64_t> distance(n + 1, INT64_MAX);
  vector<bool> visit(n + 1, false);
  distance[source] = 0;

  priority_queue<PII, vector<PII>, greater<>> pq;  // {distance, node}
  pq.emplace(0, source);

  while (!pq.empty()) {
    auto [d, u] = pq.top();
    pq.pop();
    if (visit[u]) { continue; }
    visit[u] = true;
    for (auto [v, w] : graph[u]) {
      if (distance[u] + w < distance[v]) {  // 发现更短路径
        distance[v] = distance[u] + w;
        pq.emplace(distance[v], v);
      }
    }
  }

  for (int64_t i = 1; i <= n; ++i) { cout << distance[i] << " "; }
}

int main() {
  int n, m, s;
  cin >> n >> m >> s;
  vector<vector<PII>> graph(n + 1);
  for (int i = 0; i < m; ++i) {
    int u, v;
    int64_t w;
    cin >> u >> v >> w;
    graph[u].emplace_back(v, w);
  }
  dijkstra(n, graph, s);
  return 0;
}
#include <cstdint>
#include <functional>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

using PII         = pair<int64_t, int64_t>;

const int64_t INF = 1e9;

vector<int64_t> dijkstra(vector<vector<PII>> &graph, int64_t source, vector<int64_t> &distance) {
  int64_t n = graph.size();
  distance  = vector<int64_t>(n, INF);
  vector<bool> visit(n);

  priority_queue<PII, vector<PII>, greater<>> pq;
  pq.emplace(0, source);
  distance[source] = 0;

  while (!pq.empty()) {
    auto [dis, u] = pq.top();
    pq.pop();
    if (visit[u]) { continue; }
    visit[u] = true;
    for (auto [v, w] : graph[u]) {
      if (distance[u] + w < distance[v]) {
        distance[v] = distance[u] + w;
        pq.emplace(distance[v], v);
      }
    }
  }
  return distance;
}

vector<int64_t> SPFA(const vector<vector<PII>> &graph, int64_t source) {
  int64_t n = graph.size();
  vector<int64_t> distance(n, INF);
  vector<bool> in_queue(n);    // 标志是否在队列中
  vector<int64_t> counter(n);  // 记录松弛次数，用于检测负环

  distance[source] = 0;
  queue<int64_t> q;
  q.emplace(source);
  in_queue[source] = true;
  counter[source]  = 1;

  while (!q.empty()) {
    int64_t u = q.front();
    q.pop();
    in_queue[u] = false;
    for (auto [v, w] : graph[u]) {
      if (distance[u] + w < distance[v]) {
        distance[v] = distance[u] + w;
        if (in_queue[v]) { continue; }  // 已入队
        counter[v]++;
        if (counter[v] > n - 1) {  // 有负环
          return {};
        }
        q.emplace(v);
        in_queue[v] = true;
      }
    }
  }
  return distance;
}

void johnson(int n, vector<vector<PII>> &graph) {
  // 2. 使用SPFA从s出发计算每个节点的最短距离h(v)
  vector<int64_t> h = SPFA(graph, 0);
  // 有负环, 无法使用Johnson算法
  if (h.empty()) {
    cout << -1 << "\n";
    return;
  }
  // 3. 对每条边(u, v)重新标注边权 w'(u, v) = w(u, v) + h(u) - h(v)
  for (int64_t u = 1; u <= n; ++u) {
    for (auto &[v, w] : graph[u]) { w = w + h[u] - h[v]; }
  }

  // 4. 对每个节点使用Dijkstra计算最短路径
  vector<vector<int64_t>> distances(n + 1);
  for (int64_t u = 1; u <= n; ++u) {
    dijkstra(graph, u, distances[u]);
    // 最终结果 distance(u, v) = distance'(u, v) - h(u) + h(v)
    for (int64_t v = 1; v <= n; ++v) {
      if (distances[u][v] != INF) { distances[u][v] = distances[u][v] - h[u] + h[v]; }
    }
  }

  for (int64_t u = 1; u <= n; ++u) {
    int64_t sum = 0;
    for (int64_t j = 1; j <= n; ++j) { sum += j * distances[u][j]; }
    cout << sum << "\n";
  }
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, m;
  cin >> n >> m;
  vector<vector<PII>> graph(n + 1);
  for (int64_t i = 0; i < m; ++i) {
    int64_t u, v, w;
    cin >> u >> v >> w;
    graph[u].emplace_back(v, w);
  }
  // 1. 增加虚拟节点s, 编号为0, 连接图中所有节点, 边权为0
  graph[0].resize(n);
  for (int64_t i = 1; i <= n; ++i) { graph[0][i - 1] = {i, 0}; }
  johnson(n, graph);
  return 0;
}
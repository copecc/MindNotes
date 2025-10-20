#include <cstdint>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

using PII = pair<int, int>;

vector<int> SPFA(const vector<vector<PII>> &graph, int source) {
  int n = graph.size();
  vector<int> distance(n, INT32_MAX);
  vector<bool> in_queue(n);  // 标志是否在队列中
  vector<int> counter(n);    // 记录松弛次数，用于检测负环

  distance[source] = 0;
  queue<int> q;
  q.emplace(source);
  in_queue[source] = true;
  counter[source]  = 1;

  while (!q.empty()) {
    int u = q.front();
    q.pop();
    in_queue[u] = false;
    for (auto [v, w] : graph[u]) {
      if (distance[u] + w < distance[v]) {
        distance[v] = distance[u] + w;
        if (in_queue[v]) { continue; }
        counter[v]++;
        // 有负环
        if (counter[v] > n - 1) { return {}; }
        q.emplace(v);
        in_queue[v] = true;
      }
    }
  }
  return distance;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n, m;
  cin >> n >> m;
  vector<vector<PII>> graph(n + 1);
  // 差分约束系统可行性检测
  for (int i = 0; i < m; ++i) {
    int x, y, c;
    cin >> x >> y >> c;
    graph[y].emplace_back(x, c);
  }
  // 连通超级源点
  graph[0].resize(n);
  for (int i = 1; i <= n; ++i) { graph[0][i - 1] = {i, 0}; }
  auto distance = SPFA(graph, 0);
  if (distance.empty()) {
    cout << "NO\n";
  } else {
    for (int i = 1; i <= n; ++i) { cout << distance[i] << " "; }
    cout << "\n";
  }
  return 0;
}
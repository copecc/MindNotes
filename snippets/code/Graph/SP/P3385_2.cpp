#include <cstdint>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

void solve() {
  int64_t n, m;
  cin >> n >> m;
  using PII = pair<int, int>;
  vector<vector<PII>> graph(n + 1);
  for (int64_t i = 0; i < m; ++i) {
    int64_t u, v, w;
    cin >> u >> v >> w;
    if (w < 0) {
      graph[u].emplace_back(v, w);
    } else {
      graph[u].emplace_back(v, w);
      graph[v].emplace_back(u, w);
    }
  }
  int64_t source = 1;

  vector<int64_t> distance(n + 1, INT64_MAX);
  vector<bool> in_queue(n + 1, false);  // 标志是否在队列中
  vector<int64_t> counter(n + 1, 0);    // 记录松弛次数，用于检测负环

  distance[source] = 0;
  queue<int64_t> q;
  q.emplace(source);
  in_queue[source] = true;  // 标记已入队
  counter[source]  = 1;     // 松弛次数加1

  while (!q.empty()) {
    int64_t u = q.front();
    q.pop();
    in_queue[u] = false;
    // 尝试使用该点，更新其他点的最短距离
    for (auto [v, w] : graph[u]) {
      if (distance[u] + w < distance[v]) {
        distance[v] = distance[u] + w;
        if (in_queue[v]) { continue; }  // 已入队
        counter[v]++;                   // 松弛次数加1
        if (counter[v] > n - 1) {       // 松弛次数超过n-1次，有负环
          cout << "YES\n";
          return;
        }
        q.emplace(v);        // 入队
        in_queue[v] = true;  // 标记已入队
      }
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
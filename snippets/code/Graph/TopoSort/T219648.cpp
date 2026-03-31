#include <algorithm>
#include <cstdint>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

int main() {
  int n, m;
  cin >> n >> m;

  using PII = pair<int, int>;
  vector<vector<PII>> graph(n + 1);
  vector<vector<PII>> r_graph(n + 1);
  vector<int> in_degree(n + 1, 0);
  vector<int> r_in_degree(n + 1, 0);

  for (int i = 0; i < m; i++) {
    int u, v, w;
    cin >> u >> v >> w;
    graph[u].emplace_back(v, w);
    in_degree[v]++;
    r_graph[v].emplace_back(u, w);
    r_in_degree[u]++;
  }

  vector<int> eet(n + 1, INT32_MIN);  // earliest event time, 最早事件时间
  vector<int> let(n + 1, INT32_MAX);  // latest event time, 最晚事件时间

  queue<int> q;
  for (int i = 1; i <= n; i++) {
    if (in_degree[i] == 0) {
      q.emplace(i);
      eet[i] = 0;
    }
  }

  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (auto [v, w] : graph[u]) {
      in_degree[v]--;
      if (in_degree[v] == 0) { q.emplace(v); }
      eet[v] = max(eet[v], eet[u] + w);
    }
  }

  q = {};
  for (int i = 1; i <= n; i++) {
    if (r_in_degree[i] == 0) {
      q.emplace(i);
      let[i] = eet[i];
    }
  }

  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (auto [v, w] : r_graph[u]) {
      r_in_degree[v]--;
      if (r_in_degree[v] == 0) { q.push(v); }
      let[v] = min(let[v], let[u] - w);
    }
  }

  vector<int> critical;
  for (int i = 1; i <= n; i++) {
    if (eet[i] == let[i]) { critical.push_back(i); }
  }
  sort(critical.begin(), critical.end());
  cout << critical.size() << "\n";
  for (auto v : critical) { cout << v << " "; }
  cout << "\n";
  return 0;
}
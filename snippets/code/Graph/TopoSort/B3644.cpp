#include <cstdint>
#include <iostream>
#include <queue>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n;
  cin >> n;
  vector<vector<int64_t>> g(n + 1);
  vector<int64_t> in_degree(n + 1, 0);
  for (int64_t i = 1; i <= n; i++) {
    int64_t p;
    while (cin >> p && p != 0) {
      g[i].push_back(p);
      in_degree[p]++;
    }
  }
  queue<int64_t> q;
  for (int64_t i = 1; i <= n; i++) {
    if (in_degree[i] == 0) { q.push(i); }
  }
  vector<int64_t> order;
  while (!q.empty()) {
    int64_t u = q.front();
    q.pop();
    order.push_back(u);
    for (auto v : g[u]) {
      in_degree[v]--;
      if (in_degree[v] == 0) { q.push(v); }
    }
  }
  if (order.size() != n) { return 0; }  // there is a cycle
  for (auto u : order) { cout << u << " "; }
  cout << "\n";
  return 0;
}
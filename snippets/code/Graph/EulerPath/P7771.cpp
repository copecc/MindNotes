#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int n, m;
  cin >> n >> m;
  vector<vector<pair<int, int>>> g(n + 1);  // (邻接点, 边id)
  vector<int> in_degree(n + 1, 0);
  vector<int> out_degree(n + 1, 0);
  vector<bool> used(m, false);  // 边是否被使用过

  int start = INT32_MAX;  // 起点
  for (int i = 0; i < m; ++i) {
    int u, v;
    cin >> u >> v;
    g[u].emplace_back(v, i);
    out_degree[u]++;
    in_degree[v]++;
    start = min({start, u, v});  // 记录编号最小点
  }

  // 找起点, 入度比出度大1的点为终点, 出度比入度大1的点为起点
  int start_nodes = 0, end_nodes = 0;
  for (int u = 1; u <= n; ++u) {
    if (out_degree[u] - in_degree[u] == 1) {
      start = u;
      start_nodes++;
    } else if (in_degree[u] - out_degree[u] == 1) {
      end_nodes++;
    } else if (in_degree[u] != out_degree[u]) {
      cout << "No\n";
      return 0;
    }
    sort(g[u].begin(), g[u].end(),
         greater<>());  // 为了字典序最小, 逆序存储邻接点
  }
  // 无法构成欧拉路径
  if ((start_nodes != 1 || end_nodes != 1) && (start_nodes != 0 || end_nodes != 0)) {
    cout << "No\n";
    return 0;
  }
  vector<int> path;
  auto hierholzer = [&](auto &&self, int u) -> void {
    while (!g[u].empty()) {
      auto [v, id] = g[u].back();  // 取最后一个邻接点
      g[u].pop_back();
      if (used[id]) { continue; }  // 已使用过则跳过
      used[id] = true;
      self(self, v);
    }
    path.push_back(u);  // 回溯时记录路径
  };

  hierholzer(hierholzer, start);

  if (path.size() != m + 1) {  // 未使用所有边, 图不连通
    cout << "No\n";
    return 0;
  }
  reverse(path.begin(), path.end());
  for (int v : path) { cout << v << " "; }
  cout << "\n";
  return 0;
}
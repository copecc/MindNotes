#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <utility>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int m;
  cin >> m;
  map<int, vector<pair<int, int>>> g;  // (邻接点, 边id)
  vector<bool> used(m, false);         // 边是否被使用过

  int start = INT32_MAX;  // 起点
  for (int i = 0; i < m; ++i) {
    int u, v;
    cin >> u >> v;
    g[u].emplace_back(v, i);
    g[v].emplace_back(u, i);
    start = min({start, u, v});  // 记录编号最小点
  }

  // 找起点, 奇数度点个数
  int odd = 0;
  for (const auto &[u, vs] : g) {
    if (vs.size() % 2 == 1) {
      start = odd == 0 ? u : start;  // 从编号较小的奇数度点开始
      ++odd;
    }
    sort(g[u].begin(), g[u].end(),
         greater<>());  // 为了字典序最小, 逆序存储邻接点
  }
  // 无法构成欧拉路径
  if (odd != 0 && odd != 2) { return 0; }
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
  // 未使用所有边, 图不连通
  if (path.size() != m + 1) { return 0; }

  reverse(path.begin(), path.end());
  for (int v : path) { cout << v << "\n"; }
  return 0;
}
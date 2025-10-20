#include <algorithm>
#include <climits>
#include <functional>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int n, m;
  cin >> n >> m;

  using PII = pair<int, int>;
  vector<vector<PII>> graph(n + 1);

  for (int i = 0; i < m; ++i) {
    int u, v, w;
    cin >> u >> v >> w;
    graph[u].emplace_back(v, w);
    graph[v].emplace_back(u, w);
  }

  vector<int> min_edge(n + 1, INT_MAX);  // 记录每个节点到生成树的最小边权 (1)
  vector<bool> in_mst(n + 1, false);
  priority_queue<PII, vector<PII>, greater<>> pq;  // {weight, vertex}

  min_edge[1] = 0;
  pq.emplace(0, 1);

  int mst_weight = 0;

  while (!pq.empty()) {
    auto [weight, u] = pq.top();
    pq.pop();

    if (in_mst[u]) { continue; }
    in_mst[u]   = true;
    mst_weight += weight;

    for (const auto &[v, w] : graph[u]) {
      if (!in_mst[v] && w < min_edge[v]) {  // 找到更小的边
        min_edge[v] = w;
        pq.emplace(w, v);
      }
    }
  }

  if (all_of(in_mst.begin() + 1, in_mst.end(), [](bool x) { return x; })) {
    cout << mst_weight << "\n";
  } else {
    cout << "orz\n";
  }

  return 0;
}
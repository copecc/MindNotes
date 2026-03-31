#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct Edge {
  int u, v, weight;

  bool operator<(const Edge &other) const { return weight < other.weight; }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int n, m;
  cin >> n >> m;
  vector<Edge> edges(m);
  for (int i = 0; i < m; ++i) { cin >> edges[i].u >> edges[i].v >> edges[i].weight; }

  // 并查集
  vector<int> root(n + 1);
  iota(root.begin(), root.end(), 0);

  function<int(int)> find = [&](int x) {
    if (root[x] != x) { root[x] = find(root[x]); }
    return root[x];
  };

  sort(edges.begin(), edges.end());

  int mst_weight  = 0;
  int edges_count = 0;

  for (const auto &edge : edges) {
    int root_u = find(edge.u);
    int root_v = find(edge.v);
    if (root_u != root_v) {
      mst_weight += edge.weight;
      edges_count++;
      root[root_u] = root_v;
      if (edges_count == n - 1) { break; }  // early stop
    }
  }

  if (edges_count == n - 1) {
    cout << mst_weight << "\n";
  } else {
    cout << "orz\n";
  }

  return 0;
}
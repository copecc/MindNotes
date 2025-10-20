#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int n;
  cin >> n;
  int u, v, w;
  vector<vector<pair<int, int>>> tree(n + 1);
  for (int i = 1; i < n; i++) {
    cin >> u >> v >> w;
    tree[u].emplace_back(v, w);
    tree[v].emplace_back(u, w);
  }
  int root     = 1;     // 起始节点, 任选一个节点作为根
  int farthest = root;  // 距离root最远的节点

  vector<int> dist(n + 1, 0);   // 节点到root的距离
  vector<int> last(n + 1, -1);  // x的前一个节点, 用于回溯直径路径

  std::function<void(int, int)> dfs = [&](int x, int from, int weight = 1) {
    for (const auto &[y, w] : tree[x]) {
      if (y != from) {
        last[y] = x;
        dist[y] = dist[x] + w;
        if (dist[y] > dist[farthest]) { farthest = y; }
        dfs(y, x);
      }
    }
  };
  // 第一次dfs，找到距离root最远的节点farthest
  dfs(root, -1);
  // 第二次dfs，以farthest为root，找到距离farthest最远的节点，距离即为直径
  fill(dist.begin(), dist.end(), 0);
  last[farthest] = -1;  // 重置farthest的前一个节点
  root           = farthest;
  farthest       = root;
  dfs(root, -1);

  cout << dist[farthest] << "\n";
  return 0;
}
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
  int root     = 1;  // 起始节点, 任选一个节点作为根
  int diameter = 0;  // 直径

  vector<int> max_dist(n + 1, 0);  // 以x为根的子树中，x到某个节点的最大距离

  std::function<void(int, int)> dfs = [&](int x, int from) {
    int first_max = 0, second_max = 0;  // (1)!
    for (const auto &[y, w] : tree[x]) {
      if (y != from) {
        dfs(y, x);
        if (max_dist[y] + w >= first_max) {
          second_max = first_max;
          first_max  = max_dist[y] + w;
        } else if (max_dist[y] + w > second_max) {
          second_max = max_dist[y] + w;
        }
      }
    }
    diameter    = max(diameter, first_max + second_max);
    max_dist[x] = first_max;
  };
  dfs(root, -1);

  cout << diameter << "\n";

  return 0;
}
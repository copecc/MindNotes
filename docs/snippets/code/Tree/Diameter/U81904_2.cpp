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
  vector<int> answer(n + 1, 0);    // 经过x的最长路径

  std::function<void(int, int)> dfs = [&](int x, int from) {
    for (const auto &[y, w] : tree[x]) {
      if (y != from) {
        dfs(y, x);
        answer[x]   = max(answer[x], max_dist[x] + max_dist[y] + w);  // 更新经过x的最长路径
        max_dist[x] = max(max_dist[x], max_dist[y] + w);              // 更新x到某个节点的最大距离
      }
    }
    diameter = max(diameter, answer[x]);
  };
  dfs(root, -1);

  cout << diameter << "\n";

  return 0;
}
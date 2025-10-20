#include <algorithm>
#include <cstdint>
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
  vector<vector<pair<int64_t, int64_t>>> tree(n + 1);
  for (int i = 1; i < n; ++i) {
    int64_t u, v, w;
    cin >> u >> v >> w;
    tree[u].emplace_back(v, w);
    tree[v].emplace_back(u, w);
  }
  int64_t root     = 1;
  int64_t farthest = root;  // 距离root最远的节点

  vector<int64_t> dist(n + 1, 0);   // 节点到root的距离
  vector<int64_t> last(n + 1, -1);  // x的前一个节点

  std::function<void(int64_t, int64_t)> dfs = [&](int64_t x, int64_t from) {
    for (const auto &[y, w] : tree[x]) {
      if (y != from) {
        last[y] = x;
        dist[y] = dist[x] + w;
        if (dist[y] > dist[farthest]) { farthest = y; }
        dfs(y, x);
      }
    }
  };
  dfs(root, -1);
  fill(dist.begin(), dist.end(), 0);
  last[farthest] = -1;  // 重置farthest的前一个节点
  root           = farthest;
  farthest       = root;
  dfs(root, -1);

  int64_t diameter = dist[farthest];

  // 标记直径路径上的所有点
  vector<int64_t> is_on_diameter(n + 1);
  for (int64_t i = farthest; i != -1; i = last[i]) { is_on_diameter[i] = 1; }
  // 查找不经过直径上点的最长路径
  std::function<int64_t(int64_t, int64_t)> dfs_exclude_diameter
      = [&](int64_t x, int64_t from) -> int64_t {
    int64_t max_length = 0;
    for (const auto &[y, w] : tree[x]) {
      if (y != from && !is_on_diameter[y]) {  // 只遍历不在直径上的点
        max_length = max(max_length, dfs_exclude_diameter(y, x) + w);
      }
    }
    return max_length;
  };

  int64_t left = root, right = farthest;                      // 直径的两个端点
  for (int64_t i = last[farthest]; i != root; i = last[i]) {  // 遍历直径路径上的每个节点
    int64_t max_length = dfs_exclude_diameter(i, -1);
    // 找到公共部分的右边界, 不经过直径路径，能到达距离恰好等于直径另一端的点
    if (max_length == diameter - dist[i]) { right = i; }
    // 找到公共部分的左边界
    if (max_length == dist[i] && left == root) { left = i; }
  }

  // 收集所有的直径都经过的边
  int64_t count = 0;
  for (int64_t i = right; i != left; i = last[i]) {  // (1)!
    count++;
  }
  cout << diameter << "\n" << count << "\n";

  return 0;
}
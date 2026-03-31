#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n, k;
  cin >> n >> k;
  vector<vector<int>> tree(n + 1);
  for (int i = 1; i < n; ++i) {
    int x, y;
    cin >> x >> y;
    tree[x].push_back(y);
    tree[y].push_back(x);
  }

  int m = 32 - __builtin_clz(n + 1);
  vector<vector<int>> st(n + 1, vector<int>(m, -1));
  vector<int> depth(n + 1);
  vector<int> parent(n + 1, -1);
  int root = 1;
  // 预处理倍增数组
  {
    auto dfs = [&](auto &&self, int x, int from) -> void {
      st[x][0] = from;
      for (int y : tree[x]) {
        if (y != from) {
          depth[y]  = depth[x] + 1;
          parent[y] = x;
          self(self, y, x);
        }
      }
    };
    dfs(dfs, root, -1);
    for (int j = 1; j < m; ++j) {
      for (int i = 1; i <= n; ++i) {
        if (st[i][j - 1] != -1) { st[i][j] = st[st[i][j - 1]][j - 1]; }
      }
    }
  }
  // 获取节点x的第k个祖先
  auto get_kth_ancestor = [&](int node, int k) -> int {
    for (; (k != 0) && (node != -1); k &= k - 1) { node = st[node][__builtin_ctz(k)]; }
    return node;
  };
  // 获取节点x和节点y的最近公共祖先
  auto get_lca = [&](int x, int y) -> int {
    if (depth[x] > depth[y]) { swap(x, y); }
    y = get_kth_ancestor(y, depth[y] - depth[x]);
    if (y == x) { return x; }
    for (int i = m - 1; i >= 0; --i) {
      int px = st[x][i];
      int py = st[y][i];
      if (px != py) {
        x = px;
        y = py;
      }
    }
    return st[x][0];
  };

  vector<int> values(n + 1, 0);  // 树上点差分数组, 如果有点权值，可以先将点权值加到values中
  // 处理每条路径: 点差分
  for (int i = 0; i < k; ++i) {
    int x, y;
    cin >> x >> y;
    int lca      = get_lca(x, y);
    values[x]   += 1;
    values[y]   += 1;
    values[lca] -= 1;
    if (lca != root) { values[parent[lca]] -= 1; }
  }

  // dfs累加差分值
  auto dfs = [&](auto &&self, int x, int from) -> void {
    for (int y : tree[x]) {
      if (y != from) {
        self(self, y, x);
        values[x] += values[y];  // 累加子节点的差分值
      }
    }
  };
  dfs(dfs, root, -1);

  // values数组即为每个节点的最终值
  cout << *max_element(values.begin() + 1, values.end()) << '\n';
  return 0;
}
#include <iostream>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int N, M, S;
  cin >> N >> M >> S;
  vector<vector<int>> tree(N + 1);
  for (int i = 0; i < N - 1; ++i) {
    int x, y;
    cin >> x >> y;
    tree[x].push_back(y);
    tree[y].push_back(x);
  }

  int m = 32 - __builtin_clz(N + 1);
  vector<vector<int>> st(N + 1, vector<int>(m, -1));
  vector<int> depth(N + 1);

  auto dfs = [&](auto &&self, int x, int from) -> void {
    st[x][0] = from;
    for (int y : tree[x]) {
      if (y != from) {
        depth[y] = depth[x] + 1;
        self(self, y, x);
      }
    }
  };
  dfs(dfs, S, -1);
  // 递推计算第2^j个祖先
  for (int j = 1; j < m; ++j) {
    for (int i = 1; i <= N; ++i) {
      if (st[i][j - 1] != -1) { st[i][j] = st[st[i][j - 1]][j - 1]; }
    }
  }

  auto get_kth_ancestor = [&](int node, int k) -> int {
    for (; (k != 0) && (node != -1); k &= k - 1) { node = st[node][__builtin_ctz(k)]; }
    return node;
  };

  auto get_lca = [&](int x, int y) -> int {
    if (depth[x] > depth[y]) { swap(x, y); }       // 确保x深度更小
    y = get_kth_ancestor(y, depth[y] - depth[x]);  // 使 y 和 x 在同一深度
    if (y == x) { return x; }                      // 已经是同一个祖先
    // 从第2^(m-1)个祖先开始尝试, 若祖先相同则可能到达LCA或者更高，缩小步伐
    for (int i = m - 1; i >= 0; --i) {
      int px = st[x][i];
      int py = st[y][i];
      if (px != py) {  // 若不同, 则还在更上面, 将 x 和 y 提升到它们的 2^i 级祖先
        x = px;
        y = py;
      }
    }
    return st[x][0];  // 此时x和y被提升到LCA的子节点, 返回它们的父节点即为LCA
  };

  for (int i = 0; i < M; ++i) {
    int x, y;
    cin >> x >> y;
    cout << get_lca(x, y) << "\n";
  }

  return 0;
}
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int N, M, S;
  cin >> N >> M >> S;
  vector<vector<int>> tree(N + 1);
  vector<vector<pair<int, int>>> query(N + 1);
  vector<int> answer(M);
  for (int i = 0; i < N - 1; ++i) {
    int x, y;
    cin >> x >> y;
    tree[x].push_back(y);
    tree[y].push_back(x);
  }

  for (int i = 0; i < M; ++i) {  // 记录所有查询
    int x, y;
    cin >> x >> y;
    query[x].emplace_back(y, i);
    query[y].emplace_back(x, i);
  }

  vector<bool> visited(N + 1);
  vector<int> uf_root(N + 1);
  iota(uf_root.begin(), uf_root.end(), 0);
  auto find = [&](auto &&self, int x) -> int {
    if (x != uf_root[x]) { uf_root[x] = self(self, uf_root[x]); }
    return uf_root[x];
  };

  auto dfs = [&](auto &&self, int x, int from) -> void {
    visited[x] = true;
    for (int y : tree[x]) {
      if (y != from) {
        self(self, y, x);
        uf_root[y] = x;  // 合并 y 和 x
      }
    }
    for (auto [y, idx] : query[x]) {                    // 处理所有和 x 有关的查询
      if (visited[y]) { answer[idx] = find(find, y); }  // y 已经访问过, 说明 LCA 已经确定
    }
  };
  dfs(dfs, S, -1);

  for (int i = 0; i < M; ++i) { cout << answer[i] << "\n"; }

  return 0;
}
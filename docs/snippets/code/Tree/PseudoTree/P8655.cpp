#include <iostream>
#include <queue>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int n;
  cin >> n;
  vector<vector<int>> g(n + 1);
  vector<int> in_degree(n + 1, 0);
  for (int i = 1; i <= n; ++i) {
    int u, v;
    cin >> u >> v;
    g[u].push_back(v);
    in_degree[v]++;
    g[v].push_back(u);
    in_degree[u]++;
  }

  vector<bool> visited(n + 1, false);
  queue<int> q;
  for (int i = 1; i <= n; ++i) {
    if (in_degree[i] == 1) {  // 树枝上的点入度为 1
      q.push(i);
      visited[i] = true;
    }
  }
  while (!q.empty()) {
    int x = q.front();
    q.pop();
    for (int y : g[x]) {
      if (--in_degree[y] == 1) {
        q.push(y);
        visited[y] = true;
      }
    }
  }
  for (int i = 1; i <= n; ++i) {  // 环上的点未被访问
    if (!visited[i]) { cout << i << ' '; }
  }
  return 0;
}
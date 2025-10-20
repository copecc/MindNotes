#include <algorithm>
#include <iostream>
#include <stack>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n, m;
  cin >> n >> m;

  vector<vector<int>> g(n + 1);
  for (int i = 0; i < m; ++i) {
    int u, v;
    cin >> u >> v;
    g[u].push_back(v);
  }

  vector<int> dfn(n + 1);
  vector<int> low(n + 1);     // 从该点出发所能到达的最小时间戳
  vector<int> scc_id(n + 1);  // 该点所在的SCC编号

  stack<int> st;              // 当前SCC中的点
  vector<bool> visit(n + 1);  // 表示是否在当前的SCC中

  int timer = 0, scc_count = 0;
  auto tarjan = [&](auto &&self, int u) -> void {
    dfn[u] = low[u] = ++timer;
    st.push(u);
    visit[u] = true;
    for (int v : g[u]) {
      if (!dfn[v]) {  // v未被访问, 继续DFS
        self(self, v);
        low[u] = min(low[u], low[v]);
      } else if (visit[v]) {  // v在当前SCC中, 更新low[u]
        low[u] = min(low[u], dfn[v]);
      }
    }
    if (dfn[u] == low[u]) {        // u是当前SCC的首节点
      for (int x = -1; x != u;) {  // 将该SCC中点全部出栈
        x = st.top();
        st.pop();
        visit[x]  = false;      // 将x移出当前SCC
        scc_id[x] = scc_count;  // 标记x所属的SCC编号
      }
      ++scc_count;
    }
  };

  for (int i = 1; i <= n; i++) {
    if (dfn[i] == 0) { tarjan(tarjan, i); }
  }

  vector<vector<int>> dag(scc_count);
  vector<int> scc_weight(scc_count);
  for (int i = 1; i <= n; i++) { scc_weight[scc_id[i]] += 1; }
  for (int u = 1; u <= n; ++u) {
    for (int v : g[u]) {
      if (scc_id[u] != scc_id[v]) { dag[scc_id[u]].push_back(scc_id[v]); }
    }
  }

  int star_id = -1;
  for (int u = 0; u < scc_count; ++u) {
    if (dag[u].empty()) {
      if (star_id != -1) {  // 不止一个出度为0的SCC
        star_id = -1;
        break;
      }
      star_id = u;
    }
  }
  if (star_id == -1) {
    cout << 0 << '\n';
    return 0;
  }
  cout << scc_weight[star_id] << '\n';

  return 0;
}
#include <algorithm>
#include <iostream>
#include <queue>
#include <stack>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n, m;
  cin >> n >> m;
  vector<int> weight(n + 1);
  for (int i = 1; i <= n; ++i) { cin >> weight[i]; }
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
  for (int i = 1; i <= n; i++) { scc_weight[scc_id[i]] += weight[i]; }
  for (int u = 1; u <= n; ++u) {
    for (int v : g[u]) {  // 构建缩点后的DAG
      if (scc_id[u] != scc_id[v]) { dag[scc_id[u]].push_back(scc_id[v]); }
    }
  }

  vector<int> dp(scc_count);
  int answer = 0;

  // 拓扑排序+DP
  vector<int> in_degree(scc_count);
  for (int u = 0; u < scc_count; ++u) {  // 计算入度
    for (int v : dag[u]) { in_degree[v]++; }
  }

  queue<int> q;
  for (int i = 0; i < scc_count; ++i) {
    if (in_degree[i] == 0) { q.push(i); }
    dp[i] = scc_weight[i];
  }

  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int v : dag[u]) {
      dp[v] = max(dp[v], dp[u] + scc_weight[v]);
      if (--in_degree[v] == 0) { q.push(v); }
    }
    answer = max(answer, dp[u]);
  }

  cout << answer << '\n';
  return 0;
}
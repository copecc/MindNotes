#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n;
  cin >> n;
  vector<int> est(n + 1);            // earliest start time, 最早开始时间
  vector<int> eft(n + 1);            // earliest finish time, 最早结束时间
  vector<int> time(n + 1);           // task time, 任务时间
  vector<vector<int>> graph(n + 1);  // {v1,v2,...} u depends on v1,v2,...
  vector<int> visited(n + 1, 0);     // 0:not visited, 1:visiting, 2:visited

  for (int i = 1; i <= n; ++i) {
    int no, len, task;
    cin >> no;
    cin >> time[no];
    while (cin >> task) {
      if (task == 0) { break; }
      graph[no].push_back(task);
    }
  }

  auto dfs = [&](auto &&self, int u) -> void {
    visited[u] = 1;
    est[u]     = 0;
    for (int v : graph[u]) {  // u最早开始时间 = max(依赖任务最早结束时间)
      if (visited[v] == 0) {  // 还没访问过, 需要先访问依赖任务
        self(self, v);
        est[u] = std::max(est[u], eft[v]);
      } else {  // 有些任务可能被多个任务依赖, 之前已经访问过
        est[u] = std::max(est[u], eft[v]);
      }
    }
    eft[u]     = est[u] + time[u];  // u最早结束时间 = u最早开始时间 + u任务时间
    visited[u] = 2;
  };

  for (int i = 1; i <= n; ++i) {
    if (visited[i] == 0) { dfs(dfs, i); }
  }
  cout << *max_element(eft.begin() + 1, eft.end()) << '\n';
  return 0;
}
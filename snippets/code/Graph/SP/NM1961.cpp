#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;

using PII  = pair<int64_t, int64_t>;
using TIII = tuple<int64_t, int64_t, int64_t>;

int main() {
  int64_t n, m, x;
  cin >> n >> m >> x;
  vector<vector<PII>> graph(n + 1);
  for (int64_t i = 0; i < m; ++i) {
    int64_t u, v, w;
    cin >> u >> v >> w;
    graph[u].emplace_back(v, w);
  }

  // 每个节点三种状态: 未开始异或, 已开始未结束异或, 已结束异或
  vector<vector<int64_t>> distance(n + 1, vector<int64_t>(3, INT64_MAX));
  vector<vector<bool>> visit(n + 1, vector<bool>(3, false));
  // 第一项为距离, 第二项为节点,
  // 第三项为状态(0:未开始异或,1:已开始未结束异或,2:已结束异或)
  priority_queue<TIII, vector<TIII>, greater<>> pq;
  pq.emplace(0, 1, 0);
  distance[1] = {0, 0, 0};

  while (!pq.empty()) {
    auto [dis, u, state] = pq.top();
    pq.pop();
    // 如果只需找到某一个点的最短路径, 可以在这里加一个判断相等就直接break
    if (visit[u][state]) { continue; }  // 已经处理过
    visit[u][state] = true;
    for (auto [v, w] : graph[u]) {
      if (state == 0) {                             // 未开始异或, 可以选择开始异或或者不开始异或
        if (distance[u][0] + w < distance[v][0]) {  // 不开始异或
          distance[v][0] = distance[u][0] + w;
          pq.emplace(distance[v][0], v, 0);
        }
        if (distance[u][0] + (w ^ x) < distance[v][1]) {  // 开始异或
          distance[v][1] = distance[u][0] + (w ^ x);
          pq.emplace(distance[v][1], v, 1);
        }
      } else if (state == 1) {  // 已开始未结束异或, 可以选择继续异或或者结束异或
        if (distance[u][1] + (w ^ x) < distance[v][1]) {  // 继续异或
          distance[v][1] = distance[u][1] + (w ^ x);
          pq.emplace(distance[v][1], v, 1);
        }
        if (distance[u][1] + w < distance[v][2]) {  // 结束异或
          distance[v][2] = distance[u][1] + w;
          pq.emplace(distance[v][2], v, 2);
        }
      } else if (state == 2) {  // 结束异或之后只能保持结束异或状态
        if (distance[u][2] + w < distance[v][2]) {
          distance[v][2] = distance[u][2] + w;
          pq.emplace(distance[v][2], v, 2);
        }
      }
    }
  }
  for (int64_t i = 1; i <= n; ++i) {
    int64_t ans = min({distance[i][0], distance[i][1], distance[i][2]});
    if (ans == INT64_MAX) {
      cout << -1 << " ";
    } else {
      cout << ans << " ";
    }
  }
}
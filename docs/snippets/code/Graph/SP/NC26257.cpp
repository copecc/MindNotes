#include <cstdint>
#include <functional>
#include <iostream>
#include <queue>
#include <tuple>
#include <vector>
using namespace std;

using TIII = tuple<int64_t, int64_t, int64_t>;

int64_t dijkstra(const vector<vector<TIII>> &graph, const vector<int64_t> &cost,
                 const vector<int64_t> &start_lines, int64_t source, int64_t target) {
  if (source == target) { return 0; }
  int64_t n = graph.size();
  int64_t m = cost.size();
  vector<vector<int64_t>> distance(n, vector<int64_t>(m, INT64_MAX));
  vector<vector<bool>> visit(n, vector<bool>(m, false));

  priority_queue<TIII, vector<TIII>, greater<>> pq;  // {distance, node, line_id}
  // 起点可以在不同线路的站点出发, 初始距离为该线路的花费
  for (int64_t line_id : start_lines) {
    distance[source][line_id] = cost[line_id];
    pq.emplace(distance[source][line_id], source, line_id);
  }
  while (!pq.empty()) {
    auto [d, u, u_line] = pq.top();
    pq.pop();
    if (u == target) { return d; }
    if (visit[u][u_line]) { continue; }  // 已经处理过
    visit[u][u_line] = true;

    for (auto [v, w, v_line] : graph[u]) {
      if (v_line == u_line) {                                 // 在线路内移动
        if (distance[u][u_line] + w < distance[v][v_line]) {  // 发现更短路径
          distance[v][v_line] = distance[u][u_line] + w;
          pq.emplace(distance[v][v_line], v, v_line);
        }
      } else {  // 换乘
        if (distance[u][u_line] + w + cost[v_line] < distance[v][v_line]) {
          distance[v][v_line] = distance[u][u_line] + w + cost[v_line];
          pq.emplace(distance[v][v_line], v, v_line);
        }
      }
    }
  }
  return -1;
}

int main() {
  int64_t n, m, s, t;
  cin >> n >> m >> s >> t;
  vector<vector<TIII>> graph(n + 1);  // <to, weight, line_id>
  vector<int64_t> cost(m + 1);        // 每条线的花费, 从1开始编号
  vector<int64_t> start_lines;        // 起点所在的线路
  for (int64_t i = 1; i <= m; ++i) {
    int64_t a, b, c;
    cin >> a >> b >> c;
    cost[i] = a;
    // 记录每个站点对应的线路
    for (int64_t j = 0, from; j < c; ++j) {
      int64_t to;
      cin >> to;
      if (j > 0) {  // 连接相邻站点
        graph[from].emplace_back(to, b, i);
        graph[to].emplace_back(from, b, i);
      }
      // 起点可以在不同线路的站点出发
      if (to == s) { start_lines.emplace_back(i); }
      from = to;
    }
  }

  cout << dijkstra(graph, cost, start_lines, s, t) << "\n";
  return 0;
}
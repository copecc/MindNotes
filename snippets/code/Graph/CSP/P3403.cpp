#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

int64_t dijkstra(uint64_t x, uint64_t y, uint64_t z, uint64_t h) {
  uint64_t k = min({x, y, z});
  vector<uint64_t> distance(k, UINT64_MAX);
  distance[1] = 1;  // 从 1 开始计数
  // Dijkstra
  using PII = pair<uint64_t, uint64_t>;
  priority_queue<PII, vector<PII>, greater<>> pq;  // {dist, node}
  pq.emplace(1, 1);
  while (!pq.empty()) {
    auto [dist, u] = pq.top();
    pq.pop();
    if (dist > distance[u]) { continue; }
    for (auto edge : {x, y, z}) {  // 枚举每条边, 避免建图
      uint64_t v = (u + edge) % k;
      if (distance[u] + edge < distance[v]) {
        distance[v] = distance[u] + edge;
        pq.emplace(distance[v], v);
      }
    }
  }

  int64_t result = 0;
  for (int64_t i = 0; i < k; ++i) {
    if (distance[i] <= h) { result += (h - distance[i]) / k + 1; }
  }
  return result;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  uint64_t h, x, y, z;
  cin >> h >> x >> y >> z;
  if (x == 1 || y == 1 || z == 1) {  // 有 1 可以拼凑出所有数
    cout << h << "\n";
    return 0;
  }
  cout << dijkstra(x, y, z, h) << "\n";
  return 0;
}
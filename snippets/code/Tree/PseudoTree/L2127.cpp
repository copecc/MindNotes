#include <queue>
#include <vector>
using namespace std;

class Solution {
 public:
  int maximumInvitations(vector<int> &favorite) {
    int n = favorite.size();
    vector<int> in_degree(n);
    for (int edge : favorite) { ++in_degree[edge]; }
    queue<int> q;
    for (int i = 0; i < n; ++i) {
      if (in_degree[i] == 0) { q.push(i); }
    }
    vector<int> depth(n, 1);
    while (!q.empty()) {  // (1)!
      int x = q.front();
      q.pop();
      int y    = favorite[x];
      depth[y] = max(depth[y], depth[x] + 1);
      if (--in_degree[y] == 0) { q.push(y); }
    }
    int max_ring_size = 0, sum_chain_size = 0;
    for (int i = 0; i < n; ++i) {
      if (in_degree[i] == 0) { continue; }

      in_degree[i]  = 0;  // 将基环上的点的入度标记为 0，避免重复访问
      int ring_size = 1;
      for (int v = favorite[i]; v != i; v = favorite[v]) {  // 遍历基环上的点
        in_degree[v] = 0;
        ++ring_size;
      }
      if (ring_size == 2) {  // 基环大小为 2，累加两条最长链的长度
        sum_chain_size += depth[i] + depth[favorite[i]];
      } else {  // 基环大小大于 2，只能独立成团，更新最大基环大小
        max_ring_size = max(max_ring_size, ring_size);
      }
    }
    return max(max_ring_size, sum_chain_size);
  }
};
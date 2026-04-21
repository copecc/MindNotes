#include <queue>
#include <vector>
using namespace std;

class Solution {
 public:
  bool canFinish(int numCourses, vector<vector<int>> &prerequisites) {
    vector<vector<int>> graph(numCourses);
    vector<int> indeg(numCourses, 0);
    for (const auto &edge : prerequisites) {
      int to = edge[0];
      int from = edge[1];
      graph[from].push_back(to);
      ++indeg[to];
    }

    queue<int> q;
    for (int i = 0; i < numCourses; ++i) {
      if (indeg[i] == 0) q.push(i);
    }

    int visited = 0;
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      ++visited;
      for (int v : graph[u]) {
        if (--indeg[v] == 0) q.push(v);
      }
    }
    return visited == numCourses;
  }
};

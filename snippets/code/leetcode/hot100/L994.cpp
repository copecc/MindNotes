#include <queue>
#include <vector>
using namespace std;

class Solution {
 public:
  int orangesRotting(vector<vector<int>> &grid) {
    int m = grid.size();
    int n = grid[0].size();
    queue<pair<int, int>> q;
    int fresh = 0;
    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < n; ++j) {
        if (grid[i][j] == 2) q.push({i, j});
        if (grid[i][j] == 1) ++fresh;
      }
    }

    int minutes = 0;
    const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    while (!q.empty() && fresh > 0) {
      int sz = q.size();
      ++minutes;
      while (sz--) {
        auto cur = q.front();
        q.pop();
        int x = cur.first;
        int y = cur.second;
        for (auto &d : dirs) {
          int nx = x + d[0];
          int ny = y + d[1];
          if (nx < 0 || nx >= m || ny < 0 || ny >= n || grid[nx][ny] != 1) continue;
          grid[nx][ny] = 2;
          --fresh;
          q.push({nx, ny});
        }
      }
    }
    return fresh == 0 ? minutes : -1;
  }
};

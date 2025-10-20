#include <deque>
#include <vector>
using namespace std;

class Solution {
 public:
  int minimumObstacles(vector<vector<int>> &grid) {
    int m = grid.size(), n = grid[0].size();
    array<pair<int, int>, 4> directions = {
        {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}
    };
    vector<vector<bool>> visited(m, vector<bool>(n, false));
    using TIII = tuple<int, int, int>;  // (x, y, obstacles)
    deque<TIII> dq;
    dq.emplace_back(0, 0, 0);
    visited[0][0] = true;

    while (!dq.empty()) {
      auto [x, y, obstacles] = dq.front();
      dq.pop_front();
      if (x == m - 1 && y == n - 1) { return obstacles; }
      for (auto &[dx, dy] : directions) {
        int nx = x + dx, ny = y + dy;
        if (nx >= 0 && nx < m && ny >= 0 && ny < n && !visited[nx][ny]) {
          visited[nx][ny] = true;
          if (grid[nx][ny] == 1) {  // 障碍物
            dq.emplace_back(nx, ny, obstacles + 1);
          } else {
            dq.emplace_front(nx, ny, obstacles);
          }
        }
      }
    }
    return -1;
  }
};
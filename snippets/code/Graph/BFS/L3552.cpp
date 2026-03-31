#include <deque>
#include <vector>
using namespace std;

class Solution {
 public:
  int minMoves(vector<string> &matrix) {
    int m = matrix.size(), n = matrix[0].size();
    if (matrix[0][0] == '#') { return -1; }

    using PII                = pair<int, int>;
    array<PII, 4> directions = {
        {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}
    };

    vector<vector<PII>> portals(26);  // 传送门位置
    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < n; ++j) {
        if (matrix[i][j] >= 'A' && matrix[i][j] <= 'Z') {
          portals[matrix[i][j] - 'A'].emplace_back(i, j);
        }
      }
    }

    vector<vector<int>> distance(m, vector<int>(n, INT_MAX));

    deque<PII> dq;  // (x, y)
    dq.emplace_back(0, 0);

    distance[0][0] = 0;
    while (!dq.empty()) {
      auto [x, y] = dq.front();
      dq.pop_front();
      if (x == m - 1 && y == n - 1) { return distance[x][y]; }

      // 传送门
      if (matrix[x][y] >= 'A' && matrix[x][y] <= 'Z') {
        int p = matrix[x][y] - 'A';
        for (auto &[px, py] : portals[p]) {
          if (px == x && py == y) { continue; }
          if (distance[px][py] > distance[x][y]) {
            distance[px][py] = distance[x][y];  // 传送，距离不变
            dq.emplace_front(px, py);
          }
        }
        portals[p].clear();  // 防止重复传送
      }

      for (auto &[dx, dy] : directions) {
        int nx = x + dx, ny = y + dy;
        if (nx >= 0 && nx < m && ny >= 0 && ny < n && matrix[nx][ny] != '#') {
          if (distance[nx][ny] > distance[x][y] + 1) {  // 更新距离
            distance[nx][ny] = distance[x][y] + 1;      // 普通移动，距离+1
            dq.emplace_back(nx, ny);
          }
        }
      }
    }
    return -1;
  }
};
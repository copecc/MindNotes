#include <functional>
#include <iostream>
#include <vector>
using namespace std;

bool GaussXor(vector<vector<int>> &matrix) {
  int n = matrix.size();
  // 消元
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      if (j < i && matrix[j][j] == 1) { continue; }
      if (matrix[j][i] == 1) {
        swap(matrix[i], matrix[j]);
        break;
      }
    }
    if (matrix[i][i] == 0) { continue; }
    for (int j = 0; j < n; ++j) {
      if (j != i && matrix[j][i] == 1) {
        for (int k = i; k < n + 1; ++k) { matrix[j][k] ^= matrix[i][k]; }
      }
    }
  }

  for (int i = 0; i < n; ++i) {
    if (matrix[i][i] == 0) { return false; }
  }
  return true;
}

bool BackSubstitution(const vector<vector<int>> &matrix) {
  int n = matrix.size();
  vector<int> result(n, 0);
  bool multiple_solutions = false;
  for (int i = 0; i < n; ++i) {
    if (matrix[i][i] == 0) {
      // 无解: 该行主元为0, 但常数项不为0
      if (matrix[i][n] == 1) { return false; }
      // 多解, 该行主元为0, 且常数项也为0 (自由元). 注意后续可能出现无解的情况
      multiple_solutions = true;
    }
    result[i] = matrix[i][n];
  }
  // 多解
  if (multiple_solutions) { return true; }
  // 唯一解
  return true;
}

int main() {
  int n, m;
  cin >> n >> m;
  // 构造增广矩阵: n*(n+1)
  vector<vector<int>> matrix(n, vector<int>(n + 1));
  for (int i = 0; i < n; ++i) {
    matrix[i][i] = 1;
    matrix[i][n] = 1;
  }
  for (int i = 0; i < m; ++i) {
    int a, b;
    cin >> a >> b;
    --a, --b;
    matrix[a][b] = 1;
    matrix[b][a] = 1;
  }
  int ans           = 0;
  bool has_solution = GaussXor(matrix);
  if (has_solution) {
    for (int i = 0; i < n; ++i) { ans += matrix[i][n]; }
  } else {
    ans = n;
    vector<int> ops(n, 0);
    std::function<void(int, int)> dfs = [&](int i, int res) {
      if (res >= ans) { return; }
      if (i == -1) {
        ans = min(ans, res);
        return;
      }
      if (matrix[i][i] == 0) {  // 自由元
        ops[i] = 0;
        dfs(i - 1, res);
        ops[i] = 1;
        dfs(i - 1, res + 1);
      } else {  // 主元
        int val = matrix[i][n];
        for (int j = i + 1; j < n; ++j) {
          if (matrix[i][j] == 1) { val ^= ops[j]; }
        }
        dfs(i - 1, res + val);
      }
    };
    dfs(n - 1, 0);
  }
  cout << ans << "\n";
  return 0;
}
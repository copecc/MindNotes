#include <array>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

const int MOD = 3;

// 求模 3 下逆元
inline int inv3(int x) {
  if (x == 1) { return 1; }
  if (x == 2) { return 2; }
  return 0;  // 0 无逆元
}

// 模 3 高斯消元
// matrix: n x (n+1) 增广矩阵
// x: 返回解
void GaussModular(vector<vector<int>> &matrix, vector<int> &x, int n) {
  for (int i = 0; i < n; ++i) {
    // 找主元
    int pivot = -1;
    for (int j = i; j < n; ++j) {
      if (matrix[j][i] != 0) {
        pivot = j;
        break;
      }
    }
    if (pivot == -1) { continue; }  // 主元为0, 无法消元
    swap(matrix[i], matrix[pivot]);

    // 主元归一化
    int inv = inv3(matrix[i][i]);
    for (int k = i; k <= n; ++k) { matrix[i][k] = (matrix[i][k] * inv) % MOD; }

    // 消元整列
    for (int j = 0; j < n; ++j) {
      if (j == i || matrix[j][i] == 0) { continue; }
      int factor = matrix[j][i];
      for (int k = i; k <= n; ++k) {
        matrix[j][k] = (matrix[j][k] - factor * matrix[i][k] % MOD + MOD) % MOD;
      }
    }
  }

  // 回代
  for (int i = 0; i < n; ++i) { x[i] = matrix[i][n]; }
}

// 回代输出结果
void OutputSolution(const vector<int> &x, int n, int m) {
  int total = 0;
  for (int i = 0; i < n * m; ++i) { total += x[i]; }
  cout << total << "\n";
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      int idx = i * m + j;
      for (int k = 0; k < x[idx]; ++k) { cout << i + 1 << " " << j + 1 << "\n"; }
    }
  }
}

// 构建增广矩阵
void BuildMatrix(vector<vector<int>> &matrix, int n, int m, const vector<int> &vals) {
  array<int, 4> dir_x = {0, 0, 1, -1};
  array<int, 4> dir_y = {1, -1, 0, 0};
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      int idx          = i * m + j;
      matrix[idx][idx] = 2;  // 对自己影响为 2
      // 对四个方向影响为 1
      for (int d = 0; d < 4; ++d) {
        int ni = i + dir_x[d];
        int nj = j + dir_y[d];
        if (ni < 0 || ni >= n || nj < 0 || nj >= m) { continue; }
        matrix[idx][ni * m + nj] = 1;
      }
      matrix[idx][n * m] = (MOD - vals[idx]) % MOD;
    }
  }
}

void solve() {
  int n, m;
  cin >> n >> m;
  vector<int> vals(n * m);
  for (int i = 0; i < n * m; ++i) { cin >> vals[i]; }

  vector<vector<int>> matrix(n * m, vector<int>(n * m + 1, 0));
  vector<int> x(n * m, 0);
  BuildMatrix(matrix, n, m, vals);
  GaussModular(matrix, x, n * m);
  OutputSolution(x, n, m);
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int t;
  cin >> t;
  while ((t--) != 0) { solve(); }
  return 0;
}
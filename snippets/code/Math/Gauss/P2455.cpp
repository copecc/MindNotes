#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

const double eps = 1e-9;

void GaussElimination(vector<vector<double>> &matrix) {
  int n = matrix.size();
  // 消元
  for (int i = 0; i < n; ++i) {
    // 寻找主元, 为了区分多解和无解的情况, 这里需要从第0行开始找
    int pivot = i;
    for (int j = 0; j < n; ++j) {
      // 已经消元的行不再考虑
      if (j < i && abs(matrix[j][j]) >= eps) { continue; }
      if (abs(matrix[j][i]) > abs(matrix[pivot][i])) { pivot = j; }
    }
    swap(matrix[i], matrix[pivot]);

    if (abs(matrix[i][i]) < eps) { continue; }  // 主元为0, 无法消元
    // 将主元化为1
    double major = matrix[i][i];
    for (int j = i; j < n + 1; ++j) { matrix[i][j] /= major; }
    // 消元
    for (int j = 0; j < n; ++j) {
      if (j != i) {
        double factor = matrix[j][i];
        for (int k = i; k < n + 1; ++k) { matrix[j][k] -= factor * matrix[i][k]; }
      }
    }
  }
}

void BackSubstitution(const vector<vector<double>> &matrix) {
  int n = matrix.size();
  // 回代
  vector<double> result(n, 0);
  bool multiple_solutions = false;
  for (int i = 0; i < n; ++i) {
    if (abs(matrix[i][i]) < eps) {
      // 无解: 该行主元为0, 但常数项不为0
      if (abs(matrix[i][n]) >= eps) {
        cout << -1 << '\n';
        return;
      }
      // 多解, 该行主元为0, 且常数项也为0 (自由元). 注意后续可能出现无解的情况
      multiple_solutions = true;
    }
    result[i] = matrix[i][n];
  }
  if (multiple_solutions) {  // 存在无穷多解
    cout << 0 << '\n';
    return;
  }
  if (!result.empty()) {
    int count = 1;
    for (double v : result) {
      cout << "x" << count++ << "=" << fixed << setprecision(2) << v << "\n";
    }
  }
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n;
  cin >> n;
  // 输入增广矩阵: n*(n+1)
  vector<vector<double>> matrix(n, vector<double>(n + 1));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) { cin >> matrix[i][j]; }
    cin >> matrix[i][n];
  }
  GaussElimination(matrix);
  BackSubstitution(matrix);
  return 0;
}
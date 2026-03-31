#include <numeric>
#include <vector>
using namespace std;

void GaussModular(vector<vector<int>> &matrix, int64_t mod) {
  int n = matrix.size();
  // 增广矩阵: n*(n+1), 处理系数矩阵, 使其非负
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j <= n; ++j) { matrix[i][j] = (matrix[i][j] % mod + mod) % mod; }
  }

  // 消元
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      // 已经消元的行不再考虑
      if (j < i && matrix[j][j] != 0) { continue; }
      if (matrix[j][i] != 0) {
        swap(matrix[i], matrix[j]);
        break;
      }
    }

    if (matrix[i][i] == 0) { continue; }  // 主元为0, 无法消元
    for (int j = 0; j < n; ++j) {
      if (j != i && matrix[j][i] != 0) {
        int factor   = gcd(matrix[j][i], matrix[i][i]);
        int factor_i = matrix[i][i] / factor;
        int factor_j = matrix[j][i] / factor;
        if (j < i && matrix[j][j] != 0) {
          // 如果j行有主元，那么从j列到i-1列的所有系数 * factor_j
          // 正确更新主元和自由元之间的关系
          for (int k = j; k < i; ++k) { matrix[j][k] = (matrix[j][k] * factor_i) % mod; }
        }
        // 消元
        for (int k = i; k < n + 1; ++k) {
          matrix[j][k] = ((matrix[j][k] * factor_i - matrix[i][k] * factor_j) % mod + mod) % mod;
        }
      }
    }
  }
  // 逆元表, 只适用于质数模
  vector<int64_t> inverse(mod, 0);
  inverse[1] = 1;
  for (int i = 2; i < mod; ++i) { inverse[i] = (mod - mod / i) * inverse[mod % i] % mod; }
  // 收尾, 设置主元为1
  for (int i = 0; i < n; ++i) {
    if (matrix[i][i] == 0) { continue; }
    bool free_variable = false;
    for (int j = i + 1; j < n; ++j) {
      if (matrix[i][j] != 0) {
        free_variable = true;
        break;
      }
    }
    // 如果该行有自由元，不能直接归一化主元
    if (free_variable) { continue; }
    int64_t inv  = inverse[matrix[i][i]];
    matrix[i][i] = 1;
    matrix[i][n] = (matrix[i][n] * inv) % mod;
  }
}

// 回代
void BackSubstitution(const vector<vector<double>> &matrix) {
  int n = matrix.size();
  vector<int> result(n, 0);
  bool multiple_solutions = false;
  for (int i = 0; i < n; ++i) {
    if (matrix[i][i] == 0) {
      // 无解: 该行主元为0, 但常数项不为0
      if (matrix[i][n] != 0) { return; }
      // 多解, 该行主元为0, 且常数项也为0 (自由元). 注意后续可能出现无解的情况
      multiple_solutions = true;
    }
    result[i] = matrix[i][n];
  }
  // 多解
  if (multiple_solutions) { return; }
  return;
}
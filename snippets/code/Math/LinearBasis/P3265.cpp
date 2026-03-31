#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct linear_basis_vector {
  int64_t m;              // 向量的维度
  vector<int64_t> basis;  // 线性基

  // m: 维度
  explicit linear_basis_vector(int64_t m) : m(m), basis(m, -1) {}

  // 插入向量 v 到线性基
  bool insert(int64_t i, vector<vector<double>> &vectors) {
    const double eps = 1e-5;
    for (int64_t j = 0; j < m; ++j) {
      if (abs(vectors[i][j]) < eps) { continue; }  // x 的第 j 位为 0，跳过
      if (basis[j] == -1) {                        // basis[j] 为空，插入
        basis[j] = i;
        return true;
      }
      // 消去 x 的第 i 位
      double ratio = vectors[i][j] / vectors[basis[j]][j];
      for (int64_t k = j; k < m; ++k) { vectors[i][k] -= ratio * vectors[basis[j]][k]; }
    }
    return false;  // x 被消为 0，未插入
  };
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n, m;
  cin >> n >> m;
  vector<vector<double>> vectors(n, vector<double>(m));
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) { cin >> vectors[i][j]; }
  }
  vector<int64_t> cost(n);
  for (int i = 0; i < n; ++i) { cin >> cost[i]; }
  vector<int> indices(n);
  iota(indices.begin(), indices.end(), 0);
  // 按照 cost 升序排序, cost 小的优先插入线性基
  sort(indices.begin(), indices.end(), [&](int a, int b) { return cost[a] < cost[b]; });

  linear_basis_vector lb(m);
  int64_t num = 0, total_cost = 0;
  for (int i = 0; i < n; ++i) {
    if (lb.insert(indices[i], vectors)) {
      num++;
      total_cost += cost[indices[i]];
    }
  }
  cout << num << " " << total_cost << "\n";

  return 0;
}
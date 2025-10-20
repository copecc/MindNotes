#include <cstdint>
#include <vector>
using namespace std;

using VVI = vector<vector<int64_t>>;

// 矩阵乘法，计算 a * b, 其中 a, b 都是 n*n 矩阵
VVI multiply(const VVI &a, const VVI &b, int64_t mod = 1'000'000'007) {
  int n        = a.size();
  int64_t zero = 0;  // 零元
  VVI c(n, vector<int64_t>(n, zero));

  for (int i = 0; i < n; i++) {
    for (int k = 0; k < n; k++) {
      for (int j = 0; j < n; j++) { c[i][j] = (c[i][j] + a[i][k] * b[k][j] % mod) % mod; }
    }
  }
  return c;
}

// 矩阵快速幂，计算 x^n, 其中 x 是 k*k 矩阵
VVI pow(const VVI &x, int64_t n, int64_t mod = 1'000'000'007) {
  int k        = x.size();
  int64_t zero = 0;  // 零元
  int64_t one  = 1;  // 单位元
  VVI res(k, vector<int64_t>(k, zero));
  for (int i = 0; i < k; ++i) { res[i][i] = one; }  // 初始化为单位矩阵
  VVI base = x;
  while (n > 0) {
    if ((n & 1) == 1) { res = multiply(res, base, mod); }
    base   = multiply(base, base, mod);
    n    >>= 1;
  }
  return res;
}

int fib(int n) {
  if (n == 0) { return 0; }
  VVI M = {
      {1, 1},
      {1, 0}
  };
  VVI res = pow(M, n - 1);
  return res[0][0];  // F(n) 存储在结果矩阵的 (0,0) 位置
}
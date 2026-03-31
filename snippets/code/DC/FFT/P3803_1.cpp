#include <cmath>
#include <complex>
#include <iostream>
#include <numbers>
#include <vector>
using namespace std;

void fft(vector<complex<double>> &f, int inv) {
  int n = f.size();  // n must be a power of 2
  if (n == 1) { return; }
  vector<complex<double>> even(n / 2), odd(n / 2);
  for (int i = 0; i < n / 2; i++) {
    even[i] = f[i * 2];
    odd[i]  = f[i * 2 + 1];
  }
  fft(even, inv);
  fft(odd, inv);
  double theta = 2 * numbers::pi / n;
  complex<double> wk(1), w(cos(theta), inv * sin(theta));
  for (int k = 0; k < n / 2; k++) {
    f[k]          = even[k] + wk * odd[k];
    f[k + n / 2]  = even[k] - wk * odd[k];
    wk           *= w;
  }
}

int main() {
  int n, m;
  cin >> n >> m;                            // n, m 为f(x)和g(x)的最高次
  int len = pow(2, ceil(log2(n + m + 1)));  // 调整结果的长度为2的幂次
  vector<complex<double>> f(len), g(len), h(len);
  for (int i = 0; i <= n; i++) { cin >> f[i]; }
  for (int i = 0; i <= m; i++) { cin >> g[i]; }
  fft(f, 1);
  fft(g, 1);
  for (int i = 0; i < len; i++) { h[i] = f[i] * g[i]; }  // 计算每个点的乘积
  fft(h, -1);                                            // 求h(x)的逆FFT
  // 逆变换正则化, 四舍五入取整
  for (int i = 0; i < n + m + 1; i++) { cout << round(h[i].real() / len) << " "; }
  return 0;
}
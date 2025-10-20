#include <cmath>
#include <complex>
#include <iostream>
#include <numbers>
#include <utility>
#include <vector>
using namespace std;

void bit_reverse(vector<complex<double>> &f) {
  int n = f.size();
  vector<int> reverse(n);
  for (int i = 0; i < n; ++i) {
    reverse[i] = reverse[i >> 1] >> 1;
    if ((i & 1) != 0) { reverse[i] |= n >> 1; }  // 如果最后一位是 1，则翻转成 n/2
  }
  for (int i = 0; i < n; ++i) {  // 保证每对数只翻转一次
    if (i < reverse[i]) { swap(f[i], f[reverse[i]]); }
  }
}

void fft(vector<complex<double>> &f, int inv) {
  int n = f.size();
  bit_reverse(f);
  for (int len = 2; len <= n; len *= 2) {  // 递归对应的长度
    double theta = 2 * numbers::pi / len * inv;
    complex<double> w(cos(theta), sin(theta));  // 当前单位根
    for (int i = 0; i < n; i += len) {
      complex<double> wi(1);
      for (int j = 0; j < len / 2; j++) {
        complex<double> even = f[i + j], odd = wi * f[i + j + len / 2];
        f[i + j]            = even + odd;
        f[i + j + len / 2]  = even - odd;
        wi                 *= w;
      }
    }
  }
  if (inv == -1) {  // 逆变换正则化
    for (int i = 0; i < n; i++) { f[i] /= n; }
  }
}

int main() {
  int n, m;
  cin >> n >> m;
  int len = pow(2, ceil(log2(n + m + 1)));
  vector<complex<double>> f(len), g(len), h(len);
  for (int i = 0; i <= n; i++) { cin >> f[i]; }
  for (int i = 0; i <= m; i++) { cin >> g[i]; }
  fft(f, 1);
  fft(g, 1);
  for (int i = 0; i < len; i++) { h[i] = f[i] * g[i]; }
  fft(h, -1);
  for (int i = 0; i < n + m + 1; i++) { cout << round(h[i].real()) << " "; }
  return 0;
}
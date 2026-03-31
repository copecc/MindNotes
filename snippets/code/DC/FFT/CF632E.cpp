#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <numbers>
#include <utility>
#include <vector>
using namespace std;

using cd = complex<double>;

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

// 布尔卷积（动态截断到 max_len）
vector<int> multiply(const vector<int> &a, const vector<int> &b, int max_len) {
  int n = 1;
  while (n < a.size() + b.size()) { n <<= 1; }  // 找到大于等于两多项式和的最小 2 的幂
  vector<cd> fa(n), fb(n);
  for (int i = 0; i < a.size(); i++) { fa[i] = a[i]; }
  for (int i = 0; i < b.size(); i++) { fb[i] = b[i]; }
  fft(fa, 1);
  fft(fb, 1);
  for (int i = 0; i < n; i++) { fa[i] *= fb[i]; }  // 点乘
  fft(fa, -1);                                     // 逆变换
  vector<int> res(min(n, max_len), 0);             // 截断到 max_len
  // 只关心总价是否可达, 大于 0 则视为可达
  for (int i = 0; i < res.size(); i++) { res[i] = (fa[i].real() > 0.5 ? 1 : 0); }
  return res;
}

// 多项式快速幂
vector<int> poly_pow(vector<int> base, int k, int max_sum) {
  vector<int> res = {1};
  while (k > 0) {
    if ((k & 1) != 0) { res = multiply(res, base, max_sum + 1); }
    base   = multiply(base, base, max_sum + 1);
    k    >>= 1;
  }
  return res;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int n, k;
  cin >> n >> k;
  vector<int> a(n);
  int max_a = 0;
  for (int i = 0; i < n; i++) {
    cin >> a[i];
    max_a = max(max_a, a[i]);
  }

  int max_sum = k * max_a;  // 最大可能总价

  // 构造初始多项式 F(x)
  vector<int> base(max_a + 1, 0);
  for (int i = 0; i < n; i++) { base[a[i]] = 1; }

  // 快速幂 F(x)^k
  vector<int> ans = poly_pow(base, k, max_sum);

  // 输出所有可能总价
  for (int i = 0; i <= max_sum; i++) {
    if (ans[i] != 0) { cout << i << " "; }
  }
  cout << "\n";

  return 0;
}

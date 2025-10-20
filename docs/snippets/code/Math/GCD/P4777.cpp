#include <iostream>
#include <vector>
using namespace std;

pair<int64_t, int64_t> ex_crt(const vector<int64_t> &m, const vector<int64_t> &r) {
  auto multiply = [](int64_t a, int64_t b, int64_t mod) {
    int64_t result = 0;
    a              = (a % mod + mod) % mod;
    b              = (b % mod + mod) % mod;
    while (b > 0) {
      if ((b & 1) != 0) { result = (result + a) % mod; }
      a   = (a * 2) % mod;
      b >>= 1;
    }
    return result % mod;
  };

  auto extended_gcd = [](int64_t a, int64_t b) {
    using TIII        = tuple<int64_t, int64_t, int64_t>;
    auto y_combinator = [](auto &&self, int64_t a, int64_t b) -> TIII {
      if (b == 0) { return {a, 1, 0}; }
      auto [d, x1, y1] = self(self, b, a % b);
      int64_t x        = y1;
      int64_t y        = x1 - (a / b) * y1;
      return {d, x, y};
    };
    return y_combinator(y_combinator, a, b);
  };

  int64_t x   = 0;  // 当前解
  int64_t lcm = 1;  // 当前模数的最小公倍数
  for (size_t i = 0; i < m.size(); i++) {
    int64_t a      = lcm;
    int64_t b      = m[i];
    int64_t c      = ((r[i] - x) % b + b) % b;  // 保证 c 非负
    auto [d, p, q] = extended_gcd(a, b);
    if (c % d != 0) { return {-1, -1}; }  // 无解

    int64_t t = multiply(p, c / d, b / d);  // t 是 a*t ≡ c (mod b) 的解
    x         = (x + multiply(t, a, lcm * (b / d))) % (lcm * (b / d));
    lcm       = lcm * (b / d);  // 更新最小公倍数
  }
  return {x, lcm};  // 返回解 x 和模数 M
}

int main() {
  int n;
  cin >> n;
  vector<int64_t> m(n), a(n);
  for (int i = 0; i < n; ++i) { cin >> m[i] >> a[i]; }
  auto [x, M] = ex_crt(m, a);
  cout << x << '\n';
  return 0;
}
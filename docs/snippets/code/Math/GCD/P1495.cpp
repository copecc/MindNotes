#include <iostream>
#include <vector>
using namespace std;

pair<int64_t, int64_t> crt(const vector<int64_t> &m, const vector<int64_t> &r) {
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

  int64_t lcm = 1;
  for (const auto &mi : m) { lcm *= mi; }
  int64_t x = 0;
  for (size_t i = 0; i < m.size(); i++) {
    int64_t mi = m[i];
    int64_t ri = r[i];
    int64_t ai = lcm / mi;
    // 求 ai 关于 mi 的乘法逆元
    auto [_1, inv, _2] = extended_gcd(ai, mi);
    int64_t ci         = multiply(ri, multiply(ai, inv, lcm), lcm);
    x                  = (x + ci) % lcm;
  }
  return {x, lcm};  // 返回解 x 和模数 N
}

int main() {
  int n;
  cin >> n;
  vector<int64_t> m(n), a(n);
  for (int i = 0; i < n; ++i) { cin >> m[i] >> a[i]; }
  auto [x, M] = crt(m, a);
  cout << x << '\n';
  return 0;
}
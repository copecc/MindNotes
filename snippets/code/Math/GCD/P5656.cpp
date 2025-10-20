#include <cstdint>
#include <iostream>
using namespace std;

void solve_linear_diophantine(int64_t a, int64_t b, int64_t c) {
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

  // 求解 ax + by = c 的一组特解 (x0, y0)
  auto [d, x0, y0] = extended_gcd(a, b);
  if (c % d != 0) {
    cout << -1 << '\n';  // 无解
    return;
  }
  // 特解对应 ax + by = d，转为 ax + by = c
  int64_t x  = x0 * (c / d);
  int64_t y  = y0 * (c / d);
  int64_t xd = b / d;
  int64_t yd = a / d;

  // 调整 t，使 x 变成正数的最小解
  if (x < 0) {
    int64_t t  = (1 - x + xd - 1) / xd;  // 向上取整
    x         += t * xd;
    y         -= t * yd;
  } else {
    int64_t t  = (x - 1) / xd;  // 向下取整
    x         -= t * xd;
    y         += t * yd;
  }
  // 此时是 x 的最小正数解
  if (y <= 0) {
    // y 不是正数，尝试增加 t 使 y 变成正数
    int64_t t  = (1 - y + yd - 1) / yd;  // 向上取整
    y         += t * yd;                 // 此时是 y 的最小正数解
    cout << x << ' ' << y << '\n';       // 整数解中，x 的最小正整数值，y 的最小正整数值
    return;
  }
  int64_t count = (y - 1) / yd + 1;      // 方程的正整数解个数
  int64_t x_min = x;                     // x 的最小正整数解
  int64_t x_max = x + (count - 1) * xd;  // x 的最大正整数解
  int64_t y_min = y - (count - 1) * yd;  // y 的最小正整数解
  int64_t y_max = y;                     // y 的最大正整数解
  cout << count << ' ' << x_min << ' ' << y_min << ' ' << x_max << ' ' << y_max << '\n';
}

void solve() {
  int64_t a, b, c;
  cin >> a >> b >> c;
  solve_linear_diophantine(a, b, c);
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t t = 1;
  cin >> t;
  while ((t--) != 0) { solve(); }
  return 0;
}
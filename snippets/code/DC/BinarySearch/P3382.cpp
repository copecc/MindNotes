#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>
using namespace std;

void solve() {}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n;
  double l, r;
  cin >> n >> l >> r;
  vector<double> nums(n + 1);
  for (int64_t i = n; i >= 0; --i) { cin >> nums[i]; }

  auto f = [&](double x) {
    double res = 0.0;
    for (int64_t i = n; i >= 0; --i) { res += nums[i] * pow(x, i); }
    return res;
  };

  double left = l, right = r;
  while (abs(right - left) > 1e-12) {
    double m1 = left + (right - left) / 3.0;   // 1/3 点
    double m2 = right - (right - left) / 3.0;  // 2/3 点
    if (f(m1) > f(m2)) {                       // 由于右侧单调减，左侧单调增, 极大值在左侧
      right = m2;
    } else {  // 极大值在右侧
      left = m1;
    }
  }
  cout << fixed << setprecision(5) << (left + right) / 2.0 << "\n";
  return 0;
}
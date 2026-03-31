#include <algorithm>
#include <cstdint>
#include <iostream>
using namespace std;

bool wythoff_game(int64_t a, int64_t b) {
  if (a > b) { swap(a, b); }
  int64_t k     = b - a;
  int64_t left  = (2 * a - k) * (2 * a - k);
  int64_t mid   = 5 * k * k;
  int64_t right = (2 * (a + 1) - k) * (2 * (a + 1) - k);
  // 先手必胜局面
  return mid < left || mid >= right;  // 等价于 a != floor(k * phi)
}

int main() {
  int64_t a, b;
  cin >> a >> b;
  if (a == 0 && b == 0) {  // 特判
    cout << 0 << "\n";
    return 0;
  }
  cout << (wythoff_game(a, b) ? 1 : 0) << "\n";
  return 0;
}
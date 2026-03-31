#include <iostream>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n;
  cin >> n;
  int64_t fact = 1;  // n!
  for (int64_t i = 1; i <= n; ++i) { fact = fact * i; }
  int64_t res    = fact;
  int64_t fact_i = 1;  // i!
  for (int64_t i = 1; i <= n; ++i) {
    fact_i = fact_i * i;
    if (i % 2 == 1) {
      res -= fact / fact_i;
    } else {
      res += fact / fact_i;
    }
  }
  cout << res << "\n";
  return 0;
}
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int64_t n, max_n = -1;
  vector<int64_t> n_values;
  while (cin >> n) {
    if (n == 0) { break; }
    n_values.push_back(n);
    max_n = max(max_n, n);
  }

  vector<int64_t> fib = {0, 1};
  while (true) {
    int64_t next = fib[fib.size() - 1] + fib[fib.size() - 2];
    fib.push_back(next);
    if (next > max_n) { break; }
  }

  for (int64_t n : n_values) {
    if (binary_search(fib.begin(), fib.end(), n)) {
      cout << "Second win\n";
    } else {
      cout << "First win\n";
    }
  }
  return 0;
}
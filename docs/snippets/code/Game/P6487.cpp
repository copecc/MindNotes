#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int64_t fibonacci_game(int64_t n) {
  if (n <= 2) { return 1; }
  vector<int64_t> fib = {1, 2};  // 斐波那契数列
  while (true) {
    int64_t next = fib[fib.size() - 1] + fib[fib.size() - 2];
    fib.push_back(next);
    if (next > n) { break; }
  }
  // 分解n, 找到最小的斐波那契数
  while (true) {
    auto it = std::lower_bound(fib.begin(), fib.end(), n);  // 找到第一个大于等于n的斐波那契数
    if (*it == n) { return n; }                             // n 本身就是斐波那契数
    --it;                                                   // it 指向小于 n 的最大斐波那契数
    n -= *it;
  }
}

int main() {
  int64_t n;
  cin >> n;
  cout << fibonacci_game(n) << "\n";
  return 0;
}
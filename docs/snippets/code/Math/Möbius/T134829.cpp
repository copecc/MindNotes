#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

auto mu_sieve(int64_t n) {
  static vector<int64_t> mu(n + 1);
  vector<int64_t> primes;
  vector<bool> not_prime(n + 1);
  primes.reserve(n);
  mu[1] = 1;  // 定义 μ(1) = 1
  for (int64_t x = 2; x <= n; ++x) {
    if (!not_prime[x]) {
      primes.push_back(x);
      mu[x] = -1;  // 质数的莫比乌斯函数值为 -1
    }
    for (int64_t p : primes) {
      if (x * p > n) { break; }
      not_prime[x * p] = true;
      if (x % p == 0) {  // p 是 x 的一个质因子, 则 p^2 也是 x * p 的一个质因子
        mu[x * p] = 0;   // 含有平方因子的数的莫比乌斯函数值为 0
        break;
      }
      mu[x * p] = -mu[x];  // 互质数的乘积的莫比乌斯函数值为两个数值的乘积
    }
  }
  return [&](int64_t x) { return mu[x]; };
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, m;
  cin >> n >> m;
  auto mu    = mu_sieve(max(n, m));

  auto count = [&](int64_t x) {
    int64_t res = 0;
    for (int64_t d = 1; d <= x / d; ++d) { res += mu(d) * (x / (d * d)); }
    return res;
  };
  cout << count(m) - count(n - 1) << "\n";
  return 0;
}

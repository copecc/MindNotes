#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;

// 计算 x^n % mod
int64_t pow(int64_t x, int64_t n, int64_t mod = 1'000'000'007) {
  int64_t res  = 1;
  int64_t base = x % mod;
  while (n > 0) {
    if ((n & 1) != 0) { res = (res * base) % mod; }
    base   = (base * base) % mod;
    n    >>= 1;
  }
  return res;
}

// 计算欧拉函数 φ(n)
int64_t phi(int64_t n) {
  int64_t result = n;
  for (int64_t i = 2; i * i <= n; i++) {
    if (n % i == 0) {                 // i 是 n 的一个质因子
      while (n % i == 0) { n /= i; }  // 去掉所有 i 因子, 保证每个质因子只处理一次
      result -= result / i;           // 应用公式 result *= (1 - 1/i)
    }
  }
  if (n > 1) { result -= result / n; }  // 还有一个大于 sqrt(n) 的质因子
  return result;
}

void solve() {
  int64_t n;
  cin >> n;
  while (n % 2 == 0) { n /= 2; }
  while (n % 5 == 0) { n /= 5; }

  if (n == 1) {
    cout << 1 << '\n';
    return;
  }

  int64_t euler_phi = phi(n);
  vector<int64_t> divisors;  // euler_phi的所有约数
  for (int64_t i = 1; i * i <= euler_phi; ++i) {
    if (euler_phi % i == 0) {
      divisors.push_back(i);
      if (i * i != euler_phi) { divisors.push_back(euler_phi / i); }
    }
  }
  sort(divisors.begin(), divisors.end());

  for (int64_t d : divisors) {
    if (pow(10, d, n) == 1) {  // 10^d ≡ 1 (mod n)
      cout << d << '\n';
      return;
    }
  }
}

int main() {
  ios_base::sync_with_stdio(false);
  cin.tie(nullptr);
  int t = 1;
  cin >> t;
  while ((t--) != 0) { solve(); }
  return 0;
}
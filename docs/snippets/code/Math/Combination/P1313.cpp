#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int64_t pow(int64_t x, int64_t n, int64_t mod = 1'000'000'007) {
  int64_t res  = 1;
  int64_t base = x % mod;
  while (n > 0) {
    // 如果 n 是奇数, 则需要将当前的 x 乘到结果上
    if ((n & 1) != 0) { res = (res * base) % mod; }
    base   = (base * base) % mod;
    n    >>= 1;
  }
  return res;
}

int64_t combination(int64_t n, int64_t m, int64_t mod = 1'000'000'007) {
  if (m > n || m < 0) { return 0; }
  m = std::min(m, n - m);  // 利用对称性 C(n, m) == C(n, n-m)

  // O(n) 预处理阶乘和逆元
  vector<int64_t> fact;          // 阶乘
  vector<int64_t> inverse_fact;  // 阶乘的逆元
  auto build = [&](int64_t n, int64_t mod) {
    fact.resize(n + 1, 1);
    inverse_fact.resize(n + 1, 1);
    for (int i = 2; i <= n; ++i) { fact[i] = fact[i - 1] * i % mod; }
    inverse_fact[n] = pow(fact[n], mod - 2, mod);
    for (int i = n; i > 1; --i) { inverse_fact[i - 1] = inverse_fact[i] * i % mod; }
  };
  build(n, mod);

  int64_t res = 1;
  // 计算C(n, m) = n!/(m!*(n-m)!)
  res = fact[n] * inverse_fact[m] % mod * inverse_fact[n - m] % mod;
  return res;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  const int64_t mod = 10'007;
  int64_t a, b, k, n, m;
  cin >> a >> b >> k >> n >> m;
  int64_t a_n  = pow(a, n, mod);
  int64_t b_m  = pow(b, m, mod);
  int64_t comb = combination(k, n, mod);
  cout << a_n * b_m % mod * comb % mod << "\n";
  return 0;
}
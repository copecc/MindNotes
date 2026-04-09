#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <numeric>
#include <vector>
using namespace std;

void solve() {
  int n;
  cin >> n;
  vector<int> v(n);
  for (int i = 0; i < n; i++) { cin >> v[i]; }

  map<int, int64_t> results;

  map<int, int> divisors;
  map<int, int> nextDivisors;
  for (int i = 0; i < n; i++) {
    nextDivisors.clear();
    for (const auto &[divisor, count] : divisors) { nextDivisors[gcd(divisor, v[i])] += count; }
    nextDivisors[v[i]]++;

    swap(nextDivisors, divisors);
    for (const auto &[divisor, count] : divisors) { results[divisor] += count; }
  }

  int q;
  cin >> q;
  while (q-- > 0) {
    int x;
    cin >> x;
    cout << results[x] << '\n';
  }
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t t = 1;
  // cin >> t;
  while ((t--) != 0) { solve(); }
  return 0;
}
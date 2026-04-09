#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

void solve() {
  int n;
  cin >> n;
  vector<int> v(n);
  for (int i = 0; i < n; i++) { cin >> v[i]; }

  unordered_map<int, int64_t> results;

  vector<pair<int, int64_t>> divisors;
  vector<pair<int, int64_t>> nextDivisors;
  for (int i = 0; i < n; i++) {
    nextDivisors.clear();

    // Start with subarray [i, i].
    nextDivisors.emplace_back(v[i], 1);
    // Extend previous states and merge equal gcd values.
    for (const auto &[divisor, count] : divisors) {
      int g = gcd(divisor, v[i]);
      if (nextDivisors.back().first == g) {
        nextDivisors.back().second += count;
      } else {
        nextDivisors.emplace_back(g, count);
      }
    }

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
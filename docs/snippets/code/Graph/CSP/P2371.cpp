#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

int64_t two_loop(const vector<int64_t> &a, int64_t l, int64_t r) {
  int64_t k = *min_element(a.begin(), a.end());  // 基准数
  vector<int64_t> distance(k, INT64_MAX);
  distance[0] = 0;    // 从0开始计数
  for (auto x : a) {  // 枚举每个物品
    int64_t g    = gcd(k, x);
    int64_t step = k / g;
    for (int64_t start = 0; start < g; ++start) {  // 枚举每个子环的起点
      for (int64_t t = 0; t < 2 * step; ++t) {     // 转两圈
        int64_t u = (start + t * x) % k;
        int64_t v = (u + x) % k;
        if (distance[u] != INT64_MAX && distance[u] + x < distance[v]) {
          distance[v] = distance[u] + x;
        }
      }
    }
  }

  int64_t result = 0;
  for (int64_t i = 0; i < k; ++i) {  // 枚举每个模, 计算能表示的数字个数
    if (distance[i] <= r) { result += (r - distance[i]) / k + 1; }
    if (distance[i] <= l - 1) { result -= (l - 1 - distance[i]) / k + 1; }
  }
  return result;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, l, r;
  cin >> n >> l >> r;
  vector<int64_t> a(n);
  int64_t j = 0;
  for (int64_t i = 0; i < n; ++i) {
    cin >> a[j];
    if (a[j] == 0) { continue; }  // 0 不影响方程的解
    j++;
  }
  a.resize(j);

  cout << two_loop(a, l, r) << "\n";
  return 0;
}
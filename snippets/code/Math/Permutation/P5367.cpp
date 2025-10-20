#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

int64_t cantor_expansion(const vector<int> &permutation, int64_t mod = 998'244'353) {
  int n = permutation.size();
  vector<int64_t> fenwick(n + 1, 0);  // 树状数组, 用于维护已出现的元素

  auto fenwick_update = [&](int64_t index, int64_t delta) {
    while (index <= n) {
      fenwick[index] += delta;
      index          += index & -index;
    }
  };
  auto fenwick_query = [&](int64_t index) {
    int64_t sum = 0;
    while (index > 0) {
      sum   += fenwick[index];
      index -= index & -index;
    }
    return sum;
  };
  // 从后向前处理排列元素 (1)
  int64_t result = 0;
  int64_t factor = 1;  // (n-n)!
  for (int64_t i = n; i >= 1; --i) {
    int64_t x = permutation[i - 1];
    // 查询在 x 之前的元素中有多少个已经出现过 (即右边比 x 小的元素个数)
    int64_t ci = fenwick_query(x - 1);
    result     = (result + ci * factor % mod) % mod;  // 累加当前元素的贡献
    // 标记 x 已出现
    fenwick_update(x, 1);
    factor = factor * (n - (i - 1)) % mod;  // 更新 (n-i)!
  }

  return (result + 1) % mod;
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n;
  cin >> n;
  vector<int> permutation(n);
  for (int i = 0; i < n; ++i) { cin >> permutation[i]; }
  cout << cantor_expansion(permutation) << "\n";
  return 0;
}
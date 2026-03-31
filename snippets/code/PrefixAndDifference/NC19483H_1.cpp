#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

const int mod = 1e9 + 7;

struct range_add_constant {
  int n;
  vector<int64_t> d1;

  explicit range_add_constant(int n) : n(n), d1(n + 2, 0) {}

  // [l, r] 加上常数 c
  void add(int l, int r, int64_t c) {
    d1[l]     += c;
    d1[r + 1] -= c;
  }

  // 恢复原始数组
  void recover(vector<int64_t> &nums) {
    for (int i = 1; i <= n; ++i) {
      d1[i]   = (d1[i] + d1[i - 1]) % mod;
      nums[i] = (nums[i] + d1[i]) % mod;
    }
  }
};

struct range_add_arithmetic {
  int n;
  vector<int64_t> d1, d2;

  explicit range_add_arithmetic(int n) : n(n), d1(n + 3, 0), d2(n + 3, 0) {}

  // [l, r] 加上首项 s、公差 d 的等差数列, 相当于 f(i) = s + (i - l) * d
  void add(int l, int r, int64_t s, int64_t d) {
    int64_t e  = s + (r - l) * d;
    d2[l]     += s;
    d2[l + 1] += d - s;
    d2[r + 1] -= d + e;
    d2[r + 2] += e;
  }

  // 恢复原始数组
  void recover(vector<int64_t> &nums) {
    for (int i = 1; i <= n; ++i) {
      d2[i]   = (d2[i] + d2[i - 1]) % mod;
      d1[i]   = (d1[i] + d1[i - 1] + d2[i]) % mod;
      nums[i] = (nums[i] + d1[i]) % mod;
    }
  }
};

struct range_add_quadratic {
  int n;
  vector<int64_t> d1, d2, d3;

  explicit range_add_quadratic(int n) : n(n), d1(n + 4, 0), d2(n + 4, 0), d3(n + 4, 0) {}

  // [l, r] 上加上 s + d*(i-l) + c*(i-l)^2
  void add(int l, int r, int64_t s, int64_t d, int64_t c) {
    auto f       = [&](int64_t x) { return s + d * (x - l) + c * (x - l) * (x - l); };
    int64_t e    = f(r);
    int64_t e_1  = f(r + 1);
    d3[l]       += s;
    d3[l + 1]   += d + c - 2 * s;
    d3[l + 2]   += s + c - d;
    d3[r + 1]   -= e_1;
    d3[r + 2]   += e_1 + e - 2 * c;
    d3[r + 3]   -= e;
  }

  void recover(vector<int64_t> &nums) {
    for (int i = 1; i <= n; i++) {
      d3[i]   = (d3[i] + d3[i - 1]) % mod;
      d2[i]   = (d2[i] + d2[i - 1] + d3[i]) % mod;
      d1[i]   = (d1[i] + d1[i - 1] + d2[i]) % mod;
      nums[i] = (nums[i] + d1[i]) % mod;
    }
  }
};

void solve() {
  int64_t n, m;
  cin >> n >> m;
  range_add_constant a(n);
  range_add_arithmetic b(n);
  range_add_quadratic c(n);
  for (int64_t i = 0; i < m; ++i) {
    int64_t type, pos;
    cin >> type >> pos;
    if (type == 1) {
      a.add(pos, n, 1);
    } else if (type == 2) {
      b.add(pos, n, 1, 1);
    } else {
      c.add(pos, n, 1, 2, 1);
    }
  }
  vector<int64_t> nums_a(n + 1, 0), nums_b(n + 1, 0), nums_c(n + 1, 0);
  a.recover(nums_a);
  b.recover(nums_b);
  c.recover(nums_c);
  for (int64_t i = 1; i <= n; ++i) {
    cout << (nums_a[i] + nums_b[i] + nums_c[i]) % mod;
    if (i != n) { cout << ' '; }
  }
  cout << '\n';
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t t = 1;
  cin >> t;
  while ((t--) != 0) { solve(); }
  return 0;
}
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct BIT {
  explicit BIT(int64_t n) : n(n), d(n + 2), f(n + 2) {}

  // 区间更新 [x,y] 每个位置加delta, 1 <= x <= y <= n
  void range_add(int64_t x, int64_t y, int64_t delta) {
    point_add(d, x, delta);
    point_add(d, y + 1, -delta);
    point_add(f, x, delta * (x - 1));
    point_add(f, y + 1, -delta * y);
  }

  // 查询区间和 [x, y], 1 <= x <= y <= n
  int64_t range_sum(int64_t x, int64_t y) {
    return y * sum(d, y) - sum(f, y) - ((x - 1) * sum(d, x - 1) - sum(f, x - 1));
  }

  // 单点更新 x位置加delta, 1 <= x <= n
  void point_add(vector<int64_t> &tree, int64_t x, int64_t delta) const {
    // 不查n+1位置, 上界到n即可
    for (; x <= n; x += Lowbit(x)) { tree[x] += delta; }
  }

  // 单点查询 x位置的值, 1 <= x <= n
  static int64_t sum(vector<int64_t> &tree, int64_t x) {
    int64_t ret = 0;
    for (; x > 0; x -= Lowbit(x)) { ret += tree[x]; }
    return ret;
  }

  static int64_t Lowbit(int64_t x) { return x & (-x); }

  int64_t n;          // 数组大小
  vector<int64_t> d;  // 差分树状数组, one-based indexing
  vector<int64_t> f;  // 差分树状数组, one-based indexing
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, m;
  cin >> n >> m;
  BIT bit(n);
  for (int64_t i = 1; i <= n; i++) {
    int64_t v;
    cin >> v;
    bit.range_add(i, i, v);
  }

  for (int64_t i = 0; i < m; i++) {
    int64_t op;
    cin >> op;
    if (op == 1) {
      int64_t l, r, val;
      cin >> l >> r >> val;
      bit.range_add(l, r, val);
    } else {
      int64_t l, r;
      cin >> l >> r;
      cout << bit.range_sum(l, r) << '\n';
    }
  }
  return 0;
}
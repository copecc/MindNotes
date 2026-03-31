#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct BIT {
  explicit BIT(int64_t n) : n(n), tree(n + 1) {}

  // 单点更新 x位置加delta, 1 <= x <= n
  void point_add(int64_t x, int64_t delta) {
    for (; x <= n; x += lowbit(x)) { tree[x] += delta; }
  }

  // 查询区间和 [1,x], 1 <= x <= n
  int64_t sum(int64_t x) {
    int64_t ret = 0;
    for (; x > 0; x -= lowbit(x)) { ret += tree[x]; }
    return ret;
  }

  // 查询区间和 [x,y], 1 <= x <= y <= n
  int64_t range_sum(int64_t x, int64_t y) { return sum(y) - sum(x - 1); }

  // x & (-x) 取出 x 最右边的 1
  static int64_t lowbit(int64_t x) { return x & (-x); }

  int64_t n;             // 数组大小
  vector<int64_t> tree;  // one-based indexing
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
    bit.point_add(i, v);
  }

  while ((m--) != 0) {
    int64_t op, x, y;
    cin >> op >> x >> y;
    if (op == 1) {  // 将第 x 个数加上 y
      bit.point_add(x, y);
    } else {  // 查询区间和 [x,y]
      cout << bit.range_sum(x, y) << '\n';
    }
  }
  return 0;
}
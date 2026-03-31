#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct BIT {
  explicit BIT(int64_t n) : n(n), tree(n + 2) {}

  // 区间更新 [x,y] 每个位置加delta, 1 <= x <= y <= n
  void range_add(int64_t x, int64_t y, int64_t delta) {
    point_add(x, delta);
    point_add(y + 1, -delta);
  }

  // 单点查询 x位置的值, 1 <= x <= n
  int64_t sum(int64_t x) {
    int64_t ret = 0;
    for (; x > 0; x -= lowbit(x)) { ret += tree[x]; }
    return ret;
  }

  static int64_t lowbit(int64_t x) { return x & (-x); }

  // 单点更新, 维护差分数组
  void point_add(int64_t index, int64_t delta) {
    // 不查n+1位置, 上界到n即可
    for (; index <= n; index += lowbit(index)) { tree[index] += delta; }
  }

  int64_t n;             // 数组大小
  vector<int64_t> tree;  // 差分树状数组, one-based indexing
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

  while ((m--) != 0) {
    int64_t op;
    cin >> op;
    if (op == 1) {  // 将区间 [x,y] 每个位置加 k
      int64_t x, y, k;
      cin >> x >> y >> k;
      bit.range_add(x, y, k);
    } else {  // 查询 x 位置的值
      int64_t x;
      cin >> x;
      cout << bit.sum(x) << '\n';
    }
  }
  return 0;
}
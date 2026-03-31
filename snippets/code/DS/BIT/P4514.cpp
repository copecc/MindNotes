#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct BIT {
  explicit BIT(int64_t n, int64_t m)
      : n(n),
        m(m),
        tree1(n + 1, vector<int64_t>(m + 1, 0)),
        tree2(n + 1, vector<int64_t>(m + 1, 0)),
        tree3(n + 1, vector<int64_t>(m + 1, 0)),
        tree4(n + 1, vector<int64_t>(m + 1, 0)) {}

  // 从数组初始化树状数组, nums下标从0开始
  explicit BIT(vector<vector<int64_t>> nums) : BIT(nums.size(), nums[0].size()) {
    for (int64_t i = 1; i <= n; i++) {
      for (int64_t j = 1; j <= m; j++) { point_add(i, j, nums[i - 1][j - 1]); }
    }
  }

  static int64_t lowbit(int64_t x) { return x & (-x); }

  // 单点更新 (x,y)位置加delta, 1 <= x <= n, 1 <= y <= m
  void point_add(int64_t x, int64_t y, int64_t delta) {
    int64_t delta1 = delta;
    int64_t delta2 = delta * x;
    int64_t delta3 = delta * y;
    int64_t delta4 = delta * x * y;
    for (int64_t i = x; i <= n; i += lowbit(i)) {
      for (int64_t j = y; j <= m; j += lowbit(j)) {
        tree1[i][j] += delta1;
        tree2[i][j] += delta2;
        tree3[i][j] += delta3;
        tree4[i][j] += delta4;
      }
    }
  }

  // 区间更新 (x1,y1) 到 (x2,y2) 每个位置加delta
  // 1 <= x1 <= x2 <= n, 1 <= y1 <= y2 <= m
  void range_add(int64_t x1, int64_t y1, int64_t x2, int64_t y2, int64_t delta) {
    point_add(x1, y1, delta);
    point_add(x1, y2 + 1, -delta);
    point_add(x2 + 1, y1, -delta);
    point_add(x2 + 1, y2 + 1, delta);
  }

  // 查询区间和 (1,1) 到 (x,y) , 1 <= x <= n, 1 <= y <= m
  int64_t sum(int64_t x, int64_t y) {
    int64_t ret = 0;
    for (int64_t i = x; i > 0; i -= lowbit(i)) {
      for (int64_t j = y; j > 0; j -= lowbit(j)) {
        ret += (x + 1) * (y + 1) * tree1[i][j];
        ret -= (y + 1) * tree2[i][j];
        ret -= (x + 1) * tree3[i][j];
        ret += tree4[i][j];
      }
    }
    return ret;
  }

  // 查询区间和 (x1,y1) 到 (x2,y2) , 1 <= x1 <= x2 <= n, 1 <= y1 <= y2 <= m
  int64_t range_sum(int64_t x1, int64_t y1, int64_t x2, int64_t y2) {
    return sum(x2, y2) - sum(x1 - 1, y2) - sum(x2, y1 - 1) + sum(x1 - 1, y1 - 1);
  }

 private:
  int64_t n, m;                   // 数组大小
  vector<vector<int64_t>> tree1;  // 树状数组, one-based indexing, 维护 d[i][j]
  vector<vector<int64_t>> tree2;  // 树状数组, one-based indexing, 维护 i*d[i][j]
  vector<vector<int64_t>> tree3;  // 树状数组, one-based indexing, 维护 j*d[i][j]
  vector<vector<int64_t>> tree4;  // 树状数组, one-based indexing, 维护 i*j*d[i][j]
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  char op;
  int64_t n, m;
  cin >> op >> n >> m;
  BIT bit(n, m);
  while (cin >> op) {
    if (op == 'L') {  // 将 (x1, y1), (x2, y2) 为顶点的矩形区域内的所有数字加上
                      // delta
      int64_t x1, y1, x2, y2, val;
      cin >> x1 >> y1 >> x2 >> y2 >> val;
      bit.range_add(x1, y1, x2, y2, val);
    } else if (op == 'k') {  // 查询 (x1, y1), (x2, y2)
                             // 为顶点的矩形区域内所有数字的和
      int64_t x1, y1, x2, y2;
      cin >> x1 >> y1 >> x2 >> y2;
      cout << bit.range_sum(x1, y1, x2, y2) << '\n';
    }
  }
  return 0;
}
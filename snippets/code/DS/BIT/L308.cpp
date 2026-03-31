#include <cstdint>
#include <vector>
using namespace std;

struct BIT {
  explicit BIT(int64_t n, int64_t m) : n(n), m(m), tree(n + 1, vector<int64_t>(m + 1)) {}

  // 从数组初始化树状数组, nums下标从0开始
  explicit BIT(vector<vector<int64_t>> nums) : BIT(nums.size(), nums[0].size()) {
    for (int64_t i = 1; i <= n; i++) {
      for (int64_t j = 1; j <= m; j++) { point_add(i, j, nums[i - 1][j - 1]); }
    }
  }

  // 单点更新 (x,y)位置加delta, 1 <= x <= n, 1 <= y <= m
  void point_add(int64_t x, int64_t y, int64_t delta) {
    for (int64_t i = x; i <= n; i += lowbit(i)) {
      for (int64_t j = y; j <= m; j += lowbit(j)) { tree[i][j] += delta; }
    }
  }

  // 查询区间和 (x1,y1) 到 (x2,y2) , 1 <= x1 <= x2 <= n, 1 <= y1 <= y2 <= m
  int64_t range_sum(int64_t x1, int64_t y1, int64_t x2, int64_t y2) {
    return sum(x2, y2) - sum(x1 - 1, y2) - sum(x2, y1 - 1) + sum(x1 - 1, y1 - 1);
  }

  static int64_t lowbit(int64_t x) { return x & (-x); }

  // 查询区间和 (1,1) 到 (x,y) , 1 <= x <= n, 1 <= y <= m
  int64_t sum(int64_t x, int64_t y) {
    int64_t ret = 0;
    for (int64_t i = x; i > 0; i -= lowbit(i)) {
      for (int64_t j = y; j > 0; j -= lowbit(j)) { ret += tree[i][j]; }
    }
    return ret;
  }

  int64_t n, m;                  // 数组大小
  vector<vector<int64_t>> tree;  // one-based indexing
};
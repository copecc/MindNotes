#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct BIT {
  explicit BIT(int64_t n) : n(n), tree(n + 1) {}

  void point_add(int64_t x, int64_t delta) {
    for (; x <= n; x += lowbit(x)) { tree[x] += delta; }
  }

  int64_t sum(int64_t x) {
    int64_t ret = 0;
    for (; x > 0; x -= lowbit(x)) { ret += tree[x]; }
    return ret;
  }

  static int64_t lowbit(int64_t x) { return x & (-x); }

  int64_t n;
  vector<int64_t> tree;
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, k;
  cin >> n >> k;
  // (a, b, c, i), i 为原始下标
  vector<vector<int>> nums(n + 1, vector<int>(4, 0));
  vector<int> res(n + 1, 0);  // 记录每个点的支配点数量
  for (int i = 1; i <= n; i++) {
    cin >> nums[i][0] >> nums[i][1] >> nums[i][2];
    nums[i][3] = i;  // 记录原始下标
  }
  // 按照 a, b, c 三个维度排序
  sort(nums.begin() + 1, nums.end());
  auto equal = [&](int x, int y) {
    return nums[x][0] == nums[y][0] && nums[x][1] == nums[y][1] && nums[x][2] == nums[y][2];
  };
  for (int l = 1, r = 1; l <= n; l = r) {  // 处理相同点
    while (r <= n && equal(r, l)) { ++r; }
    for (int i = l; i < r; ++i) {      // 一组内前面的点在CDQ中不会计算它之后的点
      res[nums[i][3]] += (r - i - 1);  // 先加上相同点的数量
    }
  }

  BIT bit(k);
  auto merge = [&](int left, int mid, int right) {
    int j = left;
    for (int i = mid + 1; i <= right; ++i) {
      while (j <= mid && nums[j][1] <= nums[i][1]) {
        bit.point_add(nums[j][2], 1);
        ++j;
      }
      res[nums[i][3]] += bit.sum(nums[i][2]);
    }
    // 清空 BIT
    for (int i = left; i < j; ++i) { bit.point_add(nums[i][2], -1); }
    // 根据 b 维度排序
    sort(nums.begin() + left, nums.begin() + right + 1,
         [&](const vector<int> &a, const vector<int> &b) { return a[1] < b[1]; });
  };

  auto cdq = [&](auto &&self, int left, int right) -> void {
    if (left >= right) { return; }
    int mid = (left + right) / 2;
    self(self, left, mid);
    self(self, mid + 1, right);
    merge(left, mid, right);
  };

  cdq(cdq, 1, n);

  vector<int64_t> answer(n, 0);
  for (int i = 1; i <= n; ++i) { ++answer[res[i]]; }
  for (int i = 0; i < n; ++i) { cout << answer[i] << "\n"; }
  return 0;
}
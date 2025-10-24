#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
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

  int64_t range_sum(int64_t x, int64_t y) { return sum(y) - sum(x - 1); }

  static int64_t lowbit(int64_t x) { return x & (-x); }

  int64_t n;
  vector<int64_t> tree;
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int64_t n, m;
  cin >> n >> m;

  vector<int64_t> nums(n + 1);
  for (int64_t i = 1; i <= n; i++) { cin >> nums[i]; }
  vector<int64_t> indexs(n + 1);
  iota(indexs.begin(), indexs.end(), 0);
  sort(indexs.begin() + 1, indexs.end(), [&](int64_t a, int64_t b) { return nums[a] < nums[b]; });

  struct query {
    int64_t l, r, k;
  };

  vector<query> queries(m + 1);
  for (int64_t i = 1; i <= m; i++) { cin >> queries[i].l >> queries[i].r >> queries[i].k; }
  vector<int64_t> qid(m + 1);
  iota(qid.begin(), qid.end(), 0);

  vector<int64_t> left(m + 1);
  vector<int64_t> right(m + 1);

  BIT bit(n);

  vector<int64_t> answers(m + 1);

  auto dfs = [&](auto &&self, int64_t ql, int64_t qr, int64_t l, int64_t r) -> void {
    if (ql > qr) { return; }  // 无查询
    if (l == r) {
      for (int64_t i = ql; i <= qr; i++) { answers[qid[i]] = nums[indexs[l]]; }
      return;
    }
    int64_t mid = l + ((r - l) / 2);
    // 更新BIT, 将小于等于mid的元素加入BIT
    for (int64_t i = l; i <= mid; i++) { bit.point_add(indexs[i], 1); }
    // 分配查询
    int64_t left_count = 0, right_count = 0;
    for (int64_t i = ql; i <= qr; i++) {
      int64_t cnt = bit.range_sum(queries[qid[i]].l, queries[qid[i]].r);
      if (queries[qid[i]].k <= cnt) {  // 在左半区间
        left[left_count++] = qid[i];
      } else {  // 在右半区间, 更新k值
        queries[qid[i]].k    -= cnt;
        right[right_count++]  = qid[i];
      }
    }
    // 恢复BIT
    for (int64_t i = l; i <= mid; i++) { bit.point_add(indexs[i], -1); }
    // 重新排列qid
    for (int64_t i = 0; i < left_count; i++) { qid[ql + i] = left[i]; }
    for (int64_t i = 0; i < right_count; i++) { qid[ql + i + left_count] = right[i]; }
    // 递归处理左右子区间
    self(self, ql, ql + left_count - 1, l, mid);
    self(self, ql + left_count, qr, mid + 1, r);
  };

  dfs(dfs, 1, m, 1, n);
  for (int64_t i = 1; i <= m; i++) { cout << answers[i] << "\n"; }
  return 0;
}

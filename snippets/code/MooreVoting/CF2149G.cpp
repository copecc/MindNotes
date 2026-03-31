#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

const int64_t K = 2;

struct segment_tree {
  using PII       = pair<int64_t, int64_t>;  // (value, count)
  using slot_type = array<PII, K>;
  vector<slot_type> candidates;

  explicit segment_tree(int64_t n) : candidates(n * 4) {}

  static void update(slot_type &now, int64_t candidate, int64_t count) {
    while (count > 0) {                  // 插入直到count为0
      for (int64_t i = 0; i < K; i++) {  // 1. 先尝试合并
        if (now[i].first == candidate && now[i].second > 0) {
          now[i].second += count;
          return;
        }
      }
      for (int64_t i = 0; i < K; i++) {  // 2. 尝试插入空槽
        if (now[i].second == 0) {
          now[i] = {candidate, count};
          return;
        }
      }
      // 3. 没有空槽，全部减去最小值
      int64_t min_val = count;
      for (int64_t i = 0; i < K; i++) { min_val = min(min_val, now[i].second); }
      for (int64_t i = 0; i < K; i++) { now[i].second -= min_val; }
      count -= min_val;
    }
  }

  void push_up(int64_t i) {  // 合并左右子节点的候选
    for (const auto &[candidate, count] : candidates[2 * i]) {
      if (count > 0) { update(candidates[i], candidate, count); }
    }
    for (const auto &[candidate, count] : candidates[2 * i + 1]) {
      if (count > 0) { update(candidates[i], candidate, count); }
    }
  }

  void build(int64_t i, int64_t left, int64_t right, const vector<int64_t> &nums) {
    if (left == right) {  // 叶子节点，只有一个候选
      candidates[i][0] = {nums[left], 1};
      return;
    }
    int64_t mid = left + ((right - left) / 2);
    build(2 * i, left, mid, nums);
    build(2 * i + 1, mid + 1, right, nums);
    push_up(i);
  }

  slot_type query(int64_t ql, int64_t qr, int64_t i, int64_t l, int64_t r) {
    if (ql <= l && r <= qr) { return candidates[i]; }
    int64_t mid = l + ((r - l) / 2);
    slot_type left_res, right_res;
    if (ql <= mid) { left_res = query(ql, qr, 2 * i, l, mid); }
    if (qr > mid) { right_res = query(ql, qr, 2 * i + 1, mid + 1, r); }
    // 合并左右结果
    slot_type res;
    for (const auto &[candidate, count] : left_res) {
      if (count > 0) { update(res, candidate, count); }
    }
    for (const auto &[candidate, count] : right_res) {
      if (count > 0) { update(res, candidate, count); }
    }
    return res;
  }
};

int solve() {
  int64_t n, q;
  cin >> n >> q;
  vector<int64_t> nums(n + 1);
  for (int64_t i = 1; i <= n; i++) { cin >> nums[i]; }
  vector<int64_t> sorted_nums = nums;
  sort(sorted_nums.begin() + 1, sorted_nums.end());
  auto get_rank = [&](int64_t v) {  // 获取值的排名, 从1开始
    return lower_bound(sorted_nums.begin() + 1, sorted_nums.end(), v) - sorted_nums.begin();
  };
  vector<vector<int64_t>> indexs(n + 1);
  // 记录每个排名对应的原始下标
  for (int64_t i = 1; i <= n; i++) {
    nums[i] = get_rank(nums[i]);
    indexs[nums[i]].push_back(i);
  }
  // 获取某个排名在[l, r]区间内的出现次数
  auto get_count = [&](int64_t rank, int64_t l, int64_t r) {
    return (upper_bound(indexs[rank].begin(), indexs[rank].end(), r)
            - lower_bound(indexs[rank].begin(), indexs[rank].end(), l));
  };

  segment_tree seg_tree(n);
  seg_tree.build(1, 1, n, nums);
  while ((q--) != 0) {
    int64_t l, r;
    cin >> l >> r;
    auto candidates = seg_tree.query(l, r, 1, 1, n);
    vector<int64_t> ans;
    for (const auto &[rank, _] : candidates) {
      if (get_count(rank, l, r) > (r - l + 1) / 3) { ans.push_back(rank); }
    }
    sort(ans.begin(), ans.end());
    for (int64_t x : ans) { cout << sorted_nums[x] << " "; }
    if (ans.empty()) { cout << "-1"; }
    cout << "\n";
  }
  return 0;
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
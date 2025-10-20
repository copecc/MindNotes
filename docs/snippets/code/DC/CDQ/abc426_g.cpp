#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <tuple>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n;
  cin >> n;
  vector<int> weight(n + 1), value(n + 1);
  for (int i = 1; i <= n; i++) { cin >> weight[i] >> value[i]; }
  int q;
  cin >> q;
  vector<tuple<int, int, int>> query;
  int max_c = 0;
  for (int i = 0; i < q; i++) {
    int l, r, c;
    cin >> l >> r >> c;
    query.emplace_back(l, r, c);
    max_c = max(max_c, c);
  }

  auto update = [&](const vector<int64_t> &pre, vector<int64_t> &next, int i) {
    for (int j = 0; j <= max_c; j++) {
      next[j] = pre[j];
      if (j >= weight[i]) { next[j] = max(next[j], pre[j - weight[i]] + value[i]); }
    }
  };

  vector dp(n + 1, vector<int64_t>(max_c + 1));
  vector<int64_t> ans(q);
  auto cdq = [&](auto &&cdq, int l, int r, const vector<int> &qid) -> void {
    if (l == r) {          // 只有一个物品
      for (int i : qid) {  // 此时必须有 ql == l && qr == r
        auto [ql, qr, c] = query[i];
        ans[i]           = (c >= weight[l] ? value[l] : 0);  // 能否放得下
      }
      return;
    }
    int m = (l + r) / 2;                  // 划分为 [l, m] 和 [m+1, r]
    fill(dp[m].begin(), dp[m].end(), 0);  // 初始化中点状态
    // 计算右半部分状态, 从中点向右推进, dp[i](m < i <= r) 表示 [m+1, i]
    // 物品的最优解
    for (int i = m + 1; i <= r; i++) { update(dp[i - 1], dp[i], i); }
    // 计算左半部分状态, 从中点向左推进, dp[i](l <= i <= m) 表示 [i, m]
    // 物品的最优解
    for (int j = weight[m]; j <= max_c; j++) { dp[m][j] = value[m]; }
    for (int i = m - 1; i >= l; i--) { update(dp[i + 1], dp[i], i); }
    // 分配查询
    vector<int> qid_l, qid_r;
    for (int i : qid) {
      auto [ql, qr, c] = query[i];
      if (qr <= m) {  // 全在左部分
        qid_l.push_back(i);
      } else if (ql > m) {  // 全在右部分
        qid_r.push_back(i);
      } else {  // 横跨两部分, 需要合并左右部分状态, 从左边选 j 个容量, 右边选
                // c-j 个容量
        for (int j = 0; j <= c; j++) { ans[i] = max(ans[i], dp[ql][j] + dp[qr][c - j]); }
      }
    }
    cdq(cdq, l, m, qid_l);
    cdq(cdq, m + 1, r, qid_r);
  };
  vector<int> qid(q);  // query ids
  iota(qid.begin(), qid.end(), 0);
  cdq(cdq, 1, n, qid);
  for (int i = 0; i < q; i++) { cout << ans[i] << '\n'; }
  return 0;
}
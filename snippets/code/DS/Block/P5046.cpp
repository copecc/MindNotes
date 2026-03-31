#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
using namespace std;

struct Node {
  int v, i;

  bool operator<(const Node &other) const { return v != other.v ? v < other.v : i < other.i; }
};

const int MAX_N = 100'001;
const int MAX_B = 701;
// 读取数据
int n, m;
int arr[MAX_N];
Node sorted[MAX_N];
// 分块信息
int blen, bnum;
int bi[MAX_N];
int bl[MAX_B];
int br[MAX_B];
// 树状数组
int tree[MAX_N];
// 预处理信息
int pre[MAX_N];            // 从i所在块左端点到i的逆序对数量
int suf[MAX_N];            // 从i到i所在块右端点的逆序对数量
int cnt[MAX_B][MAX_N];     // cnt[i][v]: 前i块中值小于等于v的元素数量
int64_t dp[MAX_B][MAX_B];  // dp[i][j]: 第i块到第j块的逆序对数量

inline int lowbit(int i) { return i & -i; }

inline void add(int i, int v) {
  while (i <= n) {
    tree[i] += v;
    i       += lowbit(i);
  }
}

inline int sum(int i) {
  int ret = 0;
  while (i > 0) {
    ret += tree[i];
    i   -= lowbit(i);
  }
  return ret;
}

// 计算块x的[xl,xr]和块y的[yl,yr]之间的逆序对数量
inline int count_inversion(int x, int xl, int xr, int y, int yl, int yr) {
  int ans = 0;
  for (int p1 = bl[x], p2 = bl[y], cnt = 0; p1 <= br[x]; p1++) {
    if (xl <= sorted[p1].i && sorted[p1].i <= xr) {
      while (p2 <= br[y] && sorted[p1].v > sorted[p2].v) {
        if (yl <= sorted[p2].i && sorted[p2].i <= yr) { cnt++; }
        p2++;
      }
      ans += cnt;
    }
  }
  return ans;
}

// 计算区间[l,r]的逆序对数量
int64_t query(int l, int r) {
  int64_t ans = 0;
  int lb = bi[l], rb = bi[r];
  if (lb == rb) {
    if (l == bl[lb]) {  // 区间从块左端点开始, 直接使用前缀和
      ans = pre[r];
    } else {  // 否则使用前缀和差分并减去非法部分（[bl, l-1]和[l, r]形成的逆序对数量）
      ans = pre[r] - pre[l - 1] - count_inversion(lb, bl[lb], l - 1, lb, l, r);
    }
    return ans;
  }
  // 跨块区间, 先计算左右散块部分
  ans = suf[l] + pre[r] + count_inversion(lb, l, br[lb], rb, bl[rb], r);
  // 左边散块与中间整块部分
  for (int i = l; i <= br[lb]; i++) { ans += cnt[rb - 1][arr[i]] - cnt[lb][arr[i]]; }
  // 右边散块与中间整块部分
  for (int i = bl[rb]; i <= r; i++) {
    ans += br[rb - 1] - bl[lb + 1] + 1 - (cnt[rb - 1][arr[i]] - cnt[lb][arr[i]]);
  }
  ans += dp[lb + 1][rb - 1]; // 中间整块部分
  return ans;
}

void prepare() {
  blen = static_cast<int>(sqrt(n / 4));
  blen = max(blen, 1);
  bnum = (n + blen - 1) / blen;
  for (int i = 1; i <= n; i++) { bi[i] = (i - 1) / blen + 1; }
  for (int i = 1; i <= bnum; i++) {
    bl[i] = (i - 1) * blen + 1;
    br[i] = min(i * blen, n);
    sort(sorted + bl[i], sorted + br[i] + 1);
  }

  for (int i = 1; i <= bnum; i++) {
    // 计算块内逆序对前缀
    for (int j = bl[i]; j <= br[i]; j++) {
      cnt[i][arr[j]]++;  // 统计前i块中值为arr[j]的元素数量
      if (j != bl[i]) { pre[j] = pre[j - 1] + sum(n) - sum(arr[j]); }
      add(arr[j], 1);
    }
    for (int j = bl[i]; j <= br[i]; j++) { add(arr[j], -1); }
    // 计算块内逆序对后缀
    for (int j = br[i]; j >= bl[i]; j--) {
      if (j != br[i]) { suf[j] = suf[j + 1] + sum(arr[j]); }
      add(arr[j], 1);
    }
    for (int j = bl[i]; j <= br[i]; j++) { add(arr[j], -1); }
    // 计算前i块中值小于等于j的元素数量
    int tmp = 0;
    for (int j = 1; j <= n; j++) {
      tmp       += cnt[i][j];
      cnt[i][j]  = tmp + cnt[i - 1][j];
    }
  }
  for (int l = bnum; l >= 1; l--) {
    dp[l][l] = pre[br[l]];                 // 计算块内逆序对
    for (int r = l + 1; r <= bnum; r++) {  // 计算块间逆序对
      dp[l][r] = dp[l + 1][r] + dp[l][r - 1] - dp[l + 1][r - 1]
               + count_inversion(l, bl[l], br[l], r, bl[r], br[r]);
    }
  }
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  scanf("%d %d", &n, &m);
  for (int i = 1; i <= n; i++) {
    scanf("%d", &arr[i]);
    sorted[i] = {.v = arr[i], .i = i};
  }
  prepare();
  int64_t last_ans = 0;
  for (int i = 1, l, r; i <= m; i++) {
    scanf("%d %d", &l, &r);
    l = l ^ last_ans;
    r = r ^ last_ans;
    // 确保查询区间合法
    l        = max(l, 1);
    r        = min(r, n);
    last_ans = query(l, r);
    printf("%lld\n", last_ans);
  }
  return 0;
}

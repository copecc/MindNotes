#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

const int mod = 1e9 + 7;

void solve() {
  int64_t n, m;
  cin >> n >> m;
  vector<int64_t> a(n + 2), b(n + 2), c(n + 2);

  auto recover = [&](vector<int64_t> &nums, int64_t rounds) {
    for (int64_t i = 1; i <= rounds; ++i) {
      for (int64_t i = 1; i <= n; ++i) { nums[i] = (nums[i - 1] + nums[i]) % mod; }
    }
  };
  // 因为总是对 a,b,c 操作到 n 位置，所以直接只维护开始位置的差分数组
  // 最后恢复时，先对 a 做一次前缀和，再对 b 做两次前缀和，最后对 c 做三次前缀和
  for (int64_t i = 0; i < m; ++i) {
    int64_t type, pos;
    cin >> type >> pos;
    if (type == 1) {
      a[pos] += 1;
    } else if (type == 2) {
      b[pos] += 1;  // 首项为1，公差为1, d - s = 0, 所以不需要更新 pos+1 位置
    } else {
      c[pos]     += 1;  // 首项为1，二次项系数为1, d=2, c=1
      c[pos + 1] += 1;  // pos+1 位置加上1, pos+2 位置加上0
    }
  }
  recover(a, 1);
  recover(b, 2);
  recover(c, 3);
  for (int64_t i = 1; i <= n; ++i) {
    cout << (a[i] + b[i] + c[i]) % mod;
    if (i != n) { cout << ' '; }
  }
  cout << '\n';
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
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct weighted_DSU {
  int64_t set_count;  // 当前连通分量数目

  vector<int64_t> root;    // 节点对应的根节点
  vector<int64_t> weight;  // 相对于父节点的权值

  explicit weighted_DSU(int64_t n) : set_count(n), root(n + 1), weight(n + 1, 0) {
    iota(root.begin(), root.end(), 0);
  }

  // 路径压缩, 修正到根节点的权重
  int64_t Find(int64_t x) {
    // 递归寻找根节点，更新该点到根的权重
    if (x != root[x]) {
      int64_t origin_root = root[x];
      root[x]             = Find(root[x]);
      // 父节点的权重已经更新为到根节点的权重, 更新当前节点的权重
      weight[x] += weight[origin_root];
    }
    return root[x];
  }

  // value表示x到y的权重
  bool Union(int64_t x, int64_t y, int64_t value) {
    int64_t root_x = Find(x), root_y = Find(y);
    if (root_x == root_y) { return false; }
    root[root_x] = root_y;
    --set_count;
    // 更新root_x到root_y的权重
    weight[root_x] = weight[y] - weight[x] + value;
    return true;
  }

  int64_t Query(int64_t x, int64_t y) {
    int64_t root_x = Find(x);
    int64_t root_y = Find(y);
    // 如果两个值有共同的根也就是可以计算，则算结果
    if (root_x == root_y) { return weight[x] - weight[y]; }
    return -1;  // 不在同一个并查集
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, m, q;
  cin >> n >> m >> q;
  n += 1;  // 扩展点到 n + 1 (1)
  weighted_DSU wuf(n);
  for (int64_t i = 0; i < m; i++) {
    int64_t l, r, s;
    cin >> l >> r >> s;
    wuf.Union(l, r + 1, s);
  }
  for (int64_t i = 0; i < q; i++) {
    int64_t l, r;
    cin >> l >> r;
    int64_t ans = wuf.Query(l, r + 1);
    if (ans == -1) {
      cout << "UNKNOWN\n";
    } else {
      cout << ans << "\n";
    }
  }
  return 0;
}
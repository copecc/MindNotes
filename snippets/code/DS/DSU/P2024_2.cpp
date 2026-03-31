#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct weighted_DSU {
  int64_t set_count;  // 当前连通分量数目

  vector<int64_t> root;    // 节点对应的根节点
  vector<int64_t> weight;  // 当前节点到父节点的权值

  explicit weighted_DSU(int64_t n) : set_count(n), root(n + 1), weight(n + 1) {
    iota(root.begin(), root.end(), 0);
  }

  int64_t Find(int64_t x) {
    if (x != root[x]) {
      int64_t origin_root = root[x];
      root[x]             = Find(root[x]);
      // 三种关系: 0-同类, 1-吃, 2-被吃
      weight[x] = (weight[x] + weight[origin_root]) % 3;
    }
    return root[x];
  }

  // value表示x到y的权重, 0-同类, 1-吃, 被吃关系隐含为2
  bool Union(int64_t x, int64_t y, int64_t value) {
    int64_t root_x = Find(x), root_y = Find(y);
    if (root_x == root_y) { return false; }
    // 合并x所在集合到y所在集合
    root[root_x] = root_y;
    --set_count;
    // 更新root_x到root_y的权重
    weight[root_x] = (weight[y] - weight[x] + value + 3) % 3;
    return true;
  }

  int64_t Query(int op, int64_t x, int64_t y) {
    int64_t root_x = Find(x);
    int64_t root_y = Find(y);
    // 不在同一个集合
    if (root_x != root_y) { return 1; }
    // x y 是同类, 权重相同
    if (op == 1) { return weight[x] == weight[y] ? 1 : -1; }
    // x 吃 y, 权重差为1
    if (op == 2) { return (weight[x] - weight[y] + 3) % 3 == 1 ? 1 : -1; }
    return 1;
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, k;
  cin >> n >> k;
  int64_t ans = 0;
  weighted_DSU wuf(n);
  for (int64_t q = 0; q < k; ++q) {
    int64_t op, i, j;
    cin >> op >> i >> j;
    if (i > n || j > n || op == 2 && i == j) {
      ++ans;
      continue;
    }
    int64_t res = wuf.Query(op, i, j);
    if (res == -1) {
      ++ans;
    } else {
      wuf.Union(i, j, op - 1);
    }
  }
  cout << ans << '\n';
  return 0;
}
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>
using namespace std;

struct DSU {
  int64_t set_count;  // 当前连通分量数目

  vector<int64_t> root;  // 节点对应的根节点
  vector<int64_t> size;  // 以该节点为根的集合的节点数目

  // 多一个虚拟节点
  explicit DSU(int64_t n) : set_count(n), root(n + 1), size(n + 1, 1) {
    iota(root.begin(), root.end(), 0);
  }

  int64_t Find(int64_t x) { return root[x] == x ? x : root[x] = Find(root[x]); }

  bool Union(int64_t x, int64_t y) {
    x = Find(x);
    y = Find(y);
    if (x == y) { return false; }
    // 按秩合并
    if (size[x] < size[y]) { swap(x, y); }
    root[y]  = x;
    size[x] += size[y];
    --set_count;
    return true;
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, k;
  cin >> n >> k;
  int64_t ans = 0;
  DSU uf(3 * n);
  for (int64_t q = 0; q < k; ++q) {
    int64_t op, i, j;
    cin >> op >> i >> j;
    if (i > n || j > n || op == 2 && i == j) {
      ++ans;
      continue;
    }
    // 将i拆分为i, i+n, i+2n
    // i -> i的同类, i+n -> i的被吃者, i+2n -> i的捕食者
    int64_t xi = i, xi_eat = i + n, xi_pred = i + 2 * n;
    int64_t yj = j, yj_eat = j + n, yj_pred = j + 2 * n;
    if (op == 1) {  // i j是同类
      // 冲突: i的同类是j的被吃者或捕食者
      if (uf.Find(xi) == uf.Find(yj_eat) || uf.Find(xi) == uf.Find(yj_pred)) {
        ++ans;
        continue;
      }
      // 合并同类
      uf.Union(xi, yj);
      uf.Union(xi_eat, yj_eat);
      uf.Union(xi_pred, yj_pred);
    } else {  // op == 2, i 吃 j
      // 冲突: i的同类是j的同类或j的捕食者
      if (uf.Find(xi) == uf.Find(yj) || uf.Find(xi) == uf.Find(yj_pred)) {
        ++ans;
        continue;
      }
      // i的同类是j的被吃者, i的被吃者是j的捕食者, i的捕食者是j的同类
      uf.Union(xi, yj_eat);
      uf.Union(xi_eat, yj_pred);
      uf.Union(xi_pred, yj);
    }
  }
  cout << ans << '\n';
  return 0;
}
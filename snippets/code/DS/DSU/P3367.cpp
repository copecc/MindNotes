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
  int64_t n, m;
  cin >> n >> m;
  DSU uf(n);
  for (int64_t i = 0; i < m; ++i) {
    int64_t z, x, y;
    cin >> z >> x >> y;
    if (z == 1) {
      uf.Union(x, y);
    } else {
      cout << (uf.Find(x) == uf.Find(y) ? "Y" : "N") << "\n";
    }
  }
  return 0;
}
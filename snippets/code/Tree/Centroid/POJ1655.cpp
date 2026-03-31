// This file is written in C++98 standard.

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

int n;
vector<vector<int> > tree;
vector<int> subtree_size;  // 子树节点数

// DFS 计算每个节点子树大小，并找重心
void dfs(int x, int from, int &best, int &center) {
  subtree_size[x] = 1;
  int max_child   = 0;
  for (size_t i = 0; i < tree[x].size(); ++i) {
    int y = tree[x][i];
    if (y != from) {
      dfs(y, x, best, center);
      subtree_size[x] += subtree_size[y];
      max_child        = std::max(subtree_size[y], max_child);
    }
  }
  max_child = std::max(max_child, n - subtree_size[x]);
  if (max_child < best) {
    best   = max_child;
    center = x;
  } else if (max_child == best) {
    center = std::min(x, center);
  }
}

void solve() {
  int n;
  cin >> n;
  tree         = vector<vector<int> >(n + 1);
  subtree_size = vector<int>(n + 1);
  for (int i = 0; i < n - 1; ++i) {
    int x, y;
    cin >> x >> y;
    tree[x].push_back(y);
    tree[y].push_back(x);
  }

  int best   = n;   // 最小的最大子节点数
  int center = -1;  // 重心节点, 找到编号最小的

  dfs(1, -1, best, center);
  cout << center << " " << best << "\n";
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  int t;
  cin >> t;
  while ((t--) != 0) { solve(); }
  return 0;
}
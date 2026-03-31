// This file is written in C++98 standard.

#include <algorithm>
#include <cstdio>
#include <cstring>
using namespace std;

const int MAXN = 1e5 + 5;
const int MAXM = 2e5 + 5;  // 2*(N-1)
const int LOGN = 18;       // log2(1e5) ≈ 17

struct Edge {
  int to, next;
} edges[MAXM];

int head[MAXN], tot;

// Tree & LCA
int N, M;
int st[MAXN][LOGN];
int depth[MAXN];
int values[MAXN];
int parent[MAXN];

// for add_edge
inline void add_edge(int u, int v) {
  edges[++tot].to = v;
  edges[tot].next = head[u];
  head[u]         = tot;
}

void dfs_lca(int x, int fa) {
  st[x][0] = fa;
  for (int j = 1; j < LOGN; ++j) {
    if (st[x][j - 1] != -1) {
      st[x][j] = st[st[x][j - 1]][j - 1];
    } else {
      st[x][j] = -1;
    }
  }
  for (int i = head[x]; i != 0; i = edges[i].next) {
    int y = edges[i].to;
    if (y == fa) { continue; }
    depth[y] = depth[x] + 1;
    dfs_lca(y, x);
  }
}

int get_kth_ancestor(int node, int k) {
  for (int i = 0; i < LOGN && node != -1; ++i) {
    if ((k & (1 << i)) != 0) { node = st[node][i]; }
  }
  return node;
}

int get_lca(int x, int y) {
  if (depth[x] < depth[y]) { swap(x, y); }
  int diff = depth[x] - depth[y];
  for (int i = 0; i < LOGN; ++i) {
    if ((diff & (1 << i)) != 0) { x = st[x][i]; }
  }
  if (x == y) { return x; }
  for (int i = LOGN - 1; i >= 0; --i) {
    if (st[x][i] != st[y][i]) {
      x = st[x][i];
      y = st[y][i];
    }
  }
  return st[x][0];
}

int ans;

int dfs_diff(int x, int fa) {
  for (int i = head[x]; i != 0; i = edges[i].next) {
    int y = edges[i].to;
    if (y == fa) { continue; }
    dfs_diff(y, x);
    int weight = values[y];
    if (weight == 0) {
      ans += M;
    } else if (weight == 1) {
      ans += 1;
    }
    values[x] += values[y];
  }
  return values[x];
}

int main() {
  scanf("%d%d", &N, &M);
  memset(head, 0, sizeof(head));
  memset(st, -1, sizeof(st));
  tot = 0;

  for (int i = 1; i < N; ++i) {
    int u, v;
    scanf("%d%d", &u, &v);
    add_edge(u, v);
    add_edge(v, u);
  }

  depth[1] = 0;
  dfs_lca(1, -1);

  for (int i = 0; i < M; ++i) {
    int u, v;
    scanf("%d%d", &u, &v);
    int lca      = get_lca(u, v);
    values[u]   += 1;
    values[v]   += 1;
    values[lca] -= 2;
  }

  ans = 0;
  dfs_diff(1, -1);
  printf("%d\n", ans);
  return 0;
}
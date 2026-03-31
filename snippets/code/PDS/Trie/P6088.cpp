#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
using namespace std;

struct node {
  array<int, 26> children;
  int pass;
};

vector<node> nodes;  // 所有节点

struct p_trie {
  p_trie() {
    nodes.clear();               // 清空节点
    nodes.emplace_back(node{});  // 占位, 节点编号从1开始
  }

  static int clone(int i) {
    int new_node = nodes.size();   // 新节点编号
    nodes.emplace_back(nodes[i]);  // 克隆当前节点
    return new_node;
  }

  // 插入一个数字, 返回新版本的根节点
  static int insert(const string &s, int i) {
    int new_node          = clone(i);  // 克隆当前节点
    nodes[new_node].pass += 1;
    for (int j = 0, last = new_node; j < s.size(); ++j) {
      int index                    = s[j] - 'a';
      i                            = nodes[last].children[index];
      int new_child                = clone(i);
      nodes[new_child].pass       += 1;
      nodes[last].children[index]  = new_child;
      last                         = new_child;
    }
    return new_node;
  }

  // 查询字符串出现的次数
  static int query(const string &s, int i) {
    for (char ch : s) {
      int index = ch - 'a';
      i         = nodes[i].children[index];
      if (i == 0) { return 0; }
    }
    return nodes[i].pass;
  }
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);

  int n;
  cin >> n;

  vector<vector<int>> tree(n + 1);
  map<pair<int, int>, string> edge_labels;

  for (int i = 1; i < n; ++i) {
    int u, v;
    string label;
    cin >> u >> v >> label;
    tree[u].emplace_back(v);
    tree[v].emplace_back(u);
    edge_labels[{min(u, v), max(u, v)}] = label;
  }
  // 可持久化字典树
  p_trie trie;
  vector<int> roots(n + 1);
  // 构建倍增数组和深度数组
  int m = 32 - __builtin_clz(n + 1);
  vector<vector<int>> st(n + 1, vector<int>(m, -1));
  vector<int> depth(n + 1);

  auto dfs = [&](auto &&self, int x, int from) -> void {
    st[x][0] = from;
    for (int y : tree[x]) {
      if (y != from) {
        string &label = edge_labels[{min(x, y), max(x, y)}];
        roots[y]      = p_trie::insert(label, roots[x]);
        depth[y]      = depth[x] + 1;
        self(self, y, x);
      }
    }
  };
  dfs(dfs, 1, -1);
  for (int j = 1; j < m; ++j) {
    for (int i = 1; i <= n; ++i) {
      if (st[i][j - 1] != -1) { st[i][j] = st[st[i][j - 1]][j - 1]; }
    }
  }
  // LCA相关操作
  auto get_kth_ancestor = [&](int node, int k) -> int {
    for (; (k != 0) && (node != -1); k &= k - 1) { node = st[node][__builtin_ctz(k)]; }
    return node;
  };
  auto get_lca = [&](int x, int y) -> int {
    if (depth[x] > depth[y]) { swap(x, y); }
    y = get_kth_ancestor(y, depth[y] - depth[x]);
    if (y == x) { return x; }
    for (int i = m - 1; i >= 0; --i) {
      int px = st[x][i];
      int py = st[y][i];
      if (px != py) {
        x = px;
        y = py;
      }
    }
    return st[x][0];
  };

  int q;
  cin >> q;
  for (int i = 0; i < q; ++i) {
    int u, v;
    string s;
    cin >> u >> v >> s;
    int lca          = get_lca(u, v);
    int count_in_v   = p_trie::query(s, roots[v]);
    int count_in_u   = p_trie::query(s, roots[u]);
    int count_in_lca = p_trie::query(s, roots[lca]);
    cout << count_in_v + count_in_u - count_in_lca * 2 << "\n";
  }

  return 0;
}
#include <functional>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<int> sumOfDistancesInTree(int n, vector<vector<int>> &edges) {
    vector<vector<int>> tree(n);
    for (vector<int> &edge : edges) {
      int x = edge[0], y = edge[1];
      tree[x].emplace_back(y);
      tree[y].emplace_back(x);
    }

    vector<int> size(n, 1);  // 子树的大小,包括自身
    vector<int> ans(n);

    function<void(int, int, int)> dfs1 = [&](int u, int from, int depth) {
      ans[0] += depth;  // 计算根节点的距离和
      for (int child : tree[u]) {
        if (child != from) {
          dfs1(child, u, depth + 1);
          size[u] += size[child];
        }
      }
    };
    dfs1(0, -1, 0);

    function<void(int, int)> dfs2 = [&](int u, int from) {
      for (int v : tree[u]) {
        if (v != from) {
          // 换根时size[v]个节点距离减少1,其余节点距离增加1
          ans[v] = ans[u] - size[v] + (n - size[v]);
          dfs2(v, u);
        }
      }
    };
    dfs2(0, -1);
    return ans;
  }
};
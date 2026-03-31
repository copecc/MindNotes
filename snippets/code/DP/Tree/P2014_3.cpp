#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int n, m;
  cin >> n >> m;
  vector<int> weights(n + 1, 1), values(n + 1);
  vector<vector<int>> tree(n + 1);
  for (int i = 1; i <= n; ++i) {
    int k, s;
    cin >> k >> s;
    tree[k].push_back(i);
    values[i] = s;
  }
  weights[0] = values[0] = 0;  // 根节点重量和价值为0

  vector<int> dfn(n + 1);      // DFN序对应的原始节点编号
  vector<int> size(n + 1, 0);  // 以DFN序计算的子树大小
  int timer = -1;
  // 计算DFN和子树大小
  function<int(int, int)> compute_dfn_size = [&](int u, int from) {
    int dfn_u   = ++timer;  // 当前节点在DFN序中的位置
    dfn[dfn_u]  = u;        // 记录DFN序
    size[dfn_u] = 1;        // 初始化子树大小为1
    for (int v : tree[u]) {
      if (v == from) { continue; }
      size[dfn_u] += compute_dfn_size(v, u);  // 累加子树大小
    }
    return size[dfn_u];
  };
  compute_dfn_size(0, -1);

  // dp[i][j]: 前i个节点, 在容量为j的背包中能获得的最大价值, 0-n 共n+1个节点
  vector<vector<int>> dp(n + 2, vector<int>(m + 1, 0));
  for (int i = n; i >= 0; --i) {             // 逆序枚举DFN序节点
    int u  = dfn[i];                         // 当前节点
    int sz = size[i];                        // 当前节点的子树大小
    for (int j = m; j >= weights[u]; --j) {  // 当前背包容量
      // 放入当前节点, 则子树中可以选择放入, 问题变成从i+1个节点开始选,
      // 容量为j-weights[u]的背包
      dp[i][j] = max(dp[i][j], dp[i + 1][j - weights[u]] + values[u]);
      // 不放入当前节点, 则子树中所有节点都不放入, 所以跳过子树大小个节点
      dp[i][j] = max(dp[i][j], dp[i + sz][j]);
    }
  }
  cout << dp[0][m] << "\n";

  return 0;
}
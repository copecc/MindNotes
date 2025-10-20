#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct segment_tree {
  vector<int64_t> sum;  // sum[i] 表示该节点覆盖的区间内还未使用的元素个数

  explicit segment_tree(int64_t n) : sum(n * 4) {}

  // 建树, 根节点为 1, 覆盖区间 [1, n]
  void build(int64_t node, int64_t left, int64_t right) {
    if (left == right) {
      sum[node] = 1;  // 初始时每个元素都未使用
      return;
    }
    int64_t mid = (left + right) / 2;
    build(2 * node, left, mid);
    build(2 * node + 1, mid + 1, right);
    sum[node] = sum[2 * node] + sum[2 * node + 1];
  };

  // 更新, 标记 index 位置的元素已使用
  void update(int64_t index, int64_t node, int64_t left, int64_t right) {
    if (left == right) {
      sum[node] = 0;  // 标记该元素已使用
      return;
    }
    int64_t mid = (left + right) / 2;
    if (index <= mid) {
      update(index, 2 * node, left, mid);
    } else {
      update(index, 2 * node + 1, mid + 1, right);
    }
    sum[node] = sum[2 * node] + sum[2 * node + 1];
  };

  // 查询区间和
  int64_t query(int64_t ql, int64_t qr, int64_t node, int64_t left, int64_t right) {
    if (ql <= left && right <= qr) { return sum[node]; }
    int64_t mid = (left + right) / 2;
    int64_t res = 0;
    if (ql <= mid) { res += query(ql, qr, 2 * node, left, mid); }
    if (qr > mid) { res += query(ql, qr, 2 * node + 1, mid + 1, right); }
    return res;
  };

  // 查询第 k 个未使用的元素, 并且标记该元素已使用
  int64_t query_kth(int64_t k, int64_t node, int64_t left, int64_t right) {
    if (left == right) {
      sum[node] = 0;  // 标记该元素已使用
      return left;
    }
    int64_t mid = (left + right) / 2;
    int64_t res;
    if (sum[2 * node] >= k) {  // 左子树的未使用元素个数足够, 继续往左子树找
      res = query_kth(k, 2 * node, left, mid);
    } else {  // 否则往右子树找, k 减去左子树的未使用元素个数
      res = query_kth(k - sum[2 * node], 2 * node + 1, mid + 1, right);
    }
    sum[node] = sum[2 * node] + sum[2 * node + 1];
    return res;
  };
};

void inverse_cantor_expansion(vector<int64_t> &permutation, int64_t k) {
  int64_t n = permutation.size();
  segment_tree tree(n);
  tree.build(1, 1, n);

  // 计算当前排列的康托展开排名
  for (int64_t i = 0; i < n; ++i) {
    int64_t x = permutation[i];
    if (x == 1) {
      permutation[i] = 0;
    } else {
      // 查询在 x 之前的元素中有多少个未使用
      permutation[i] = tree.query(1, x - 1, 1, 1, n);
    }
    tree.update(x, 1, 1, n);  // 标记 x 已使用
  }
  permutation[n - 1] += k;  // 对最后一个位置的 ci 值加上 k
  // 处理进位
  for (int64_t i = n - 1; i > 0; --i) {
    permutation[i - 1] += permutation[i] / (n - i);
    permutation[i]     %= (n - i);
  }

  // 重新建树, 用于根据 ci 值构造排列
  tree.build(1, 1, n);
  for (int64_t i = 0; i < n; ++i) {
    int64_t ci = permutation[i];
    // 查询第 ci+1 个未使用的元素，并标记该元素已使用
    permutation[i] = tree.query_kth(ci + 1, 1, 1, n);
  }
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t n, k;
  cin >> n >> k;
  vector<int64_t> permutation(n);
  for (int64_t i = 0; i < n; ++i) { cin >> permutation[i]; }
  inverse_cantor_expansion(permutation, k);
  for (int64_t i = 0; i < n; ++i) {
    if (i > 0) { cout << " "; }
    cout << permutation[i];
  }
  cout << "\n";
  return 0;
}
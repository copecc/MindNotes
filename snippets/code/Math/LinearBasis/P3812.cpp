#include <cstdint>
#include <iostream>
#include <vector>
using namespace std;

struct linear_basis {
  int64_t bit;            // 线性基的位数
  vector<int64_t> basis;  // 线性基

  bool has_zero = false;  // 原数组是否能表示 0

  explicit linear_basis(int64_t bit = 64) : bit(bit), basis(bit) {}

  // 插入 x 到线性基, 如果插入成功返回 true, 否则返回 false
  // 插入失败表示 x 可以由线性基表示, 即 x 被消为 0, 原数组可以表示 0
  bool insert(int64_t x) {
    for (int64_t i = bit - 1; i >= 0; --i) {
      // x 的第 i 位为 0，跳过
      if ((x >> i & 1) == 0) { continue; }
      if (basis[i] == 0) {  // basis[i] 为空，插入
        basis[i] = x;
        return true;
      }
      x ^= basis[i];  // 消去 x 的第 i 位
    }
    has_zero = true;  // x 被消为 0, 原数组能表示 0
    return false;     // x 被消为 0, 未插入
  };

  // 求线性基中的最大异或和
  int64_t max_xor() const {
    int64_t res = 0;
    for (int64_t i = bit - 1; i >= 0; --i) { res = max(res, res ^ basis[i]); }
    return res;
  };

  // 求线性基中的最小异或和
  int64_t min_xor() const {
    for (int64_t i = 0; i < bit; ++i) {
      if (basis[i] != 0) { return basis[i]; }  // 最小非零异或和
    }
    return 0LL;  // 线性基为空, 最小异或和为 0
  };

  // 求线性基的秩
  int64_t rank() const {
    int64_t res = 0;
    for (int64_t i = 0; i < bit; ++i) { res += static_cast<int64_t>(basis[i] != 0); }
    return res;
  };

  // 求线性基能表示的数的个数: 2^rank
  int64_t size() const { return 1LL << rank(); };

  // 检查 x 是否可以由线性基表示
  bool check(int64_t x) const {
    for (int64_t i = bit - 1; i >= 0; --i) {
      if ((x >> i & 1) == 0) { continue; }
      if (basis[i] == 0) { return false; }  // basis[i] 为空，无法表示
      x ^= basis[i];                        // 消去 x 的第 i 位
    }
    return true;  // x 被消为 0，可以表示
  };
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n;
  cin >> n;
  linear_basis lb(64);
  for (int64_t i = 0; i < n; ++i) {
    int64_t x;
    cin >> x;
    lb.insert(x);
  }
  cout << lb.max_xor() << "\n";
  return 0;
}
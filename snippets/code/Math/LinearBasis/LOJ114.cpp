#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

struct guassian_basis {
  vector<int64_t> basis;  // 线性基

  bool has_zero = false;  // 是否能表示 0

  explicit guassian_basis(const vector<int64_t> &nums, int64_t bit = 64) : basis(nums) {
    int64_t n   = nums.size();
    int64_t len = 0;  // 线性基的长度
    // 高斯消元构建线性基
    for (int64_t i = bit - 1; i >= 0; --i) {
      for (int64_t j = len; j < n; ++j) {
        if ((basis[j] >> i & 1) != 0) {  // 找到第 i 位为 1 的数
          swap(basis[j], basis[len]);    // 将其放到前面
          break;
        }
      }
      if ((basis[len] >> i & 1) == 0) { continue; }  // 没有找到，跳过
      for (int64_t j = 0; j < n; ++j) {
        // 消去第 i 位
        if (j != len && (basis[j] >> i & 1) != 0) { basis[j] ^= basis[len]; }
      }
      len++;  // 增加线性基的长度
    }

    basis.resize(len);
    has_zero = len < n;
  }

  // 求k大异或和
  int64_t kth_xor(int64_t k) const {
    if (has_zero && k == 1) { return 0LL; }
    if (has_zero) { k--; }                            // 能表示 0，k 减 1
    if (k >= (1LL << basis.size())) { return -1LL; }  // 超出范围
    int64_t res = 0;
    for (int64_t i = basis.size() - 1, j = 0; i >= 0; --i, ++j) {
      if ((k >> j & 1) != 0) { res ^= basis[i]; }  // 第 i 位为 1，加入 basis[i]
    }
    return res;
  }

  // 求线性基中的最大异或和等信息和普通消元法类似
};

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n;
  cin >> n;
  vector<int64_t> nums(n);
  for (int64_t i = 0; i < n; ++i) { cin >> nums[i]; }
  guassian_basis gb(nums);

  int m;
  cin >> m;
  for (int64_t i = 0; i < m; ++i) {
    int64_t k;
    cin >> k;
    cout << gb.kth_xor(k) << "\n";
  }
}
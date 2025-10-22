#include <cstdint>
#include <vector>
using namespace std;

auto rad_sieve = [](int64_t n) {
  static vector<int> core(n + 1);
  for (int i = 1; i <= n; ++i) {
    if (core[i] == 0) {  // i 不含完全平方因子，可以作为 core 值
      for (int j = 1; i * j * j <= n; ++j) { core[i * j * j] = i; }
    }
  }
  return [&](int64_t x) { return core[x]; };
}(10'000);

class Solution {
  int64_t rad(int64_t n) {
    for (int64_t i = 2; i * i <= n; i++) {
      while (n % (i * i) == 0) { n /= i * i; }
    }
    return n;
  }

 public:
  long long maximumSum(vector<int> &nums) {
    int n = nums.size();
    {  // 枚举每个数的 rad 值，然后统计各个 rad 值对应的和，最后取最大值
      vector<int64_t> sum(n + 1);
      for (int i = 0; i < n; i++) { sum[rad(i + 1)] += nums[i]; }
      return ranges::max(sum);
    }
    {  // 使用预处理的 rad_sieve 来加速计算
      int64_t ans = 0;
      vector<int64_t> sum(n + 1, 0);
      for (int i = 1; i <= n; ++i) {
        sum[rad_sieve(i)] += nums[i - 1];
        ans                = max(ans, sum[rad_sieve(i)]);
      }
      return ans;
    }
  }
};

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

int main() {
  int n, k;
  while (cin >> n >> k) {
    if (n == 0 && k == 0) { break; }
    vector<pair<int, int>> nums(n);
    for (int i = 0; i < n; i++) { cin >> nums[i].first; }
    for (int i = 0; i < n; i++) { cin >> nums[i].second; }

    auto check = [&](double x) {
      vector<double> diffs(n);
      for (int i = 0; i < n; ++i) { diffs[i] = nums[i].first - x * nums[i].second; }
      sort(diffs.rbegin(), diffs.rend());
      double sum = 0;
      for (int i = 0; i < n - k; ++i) { sum += diffs[i]; }
      return sum >= 0;
    };

    double left = 0, right = 1e9 + 1;
    for (int i = 0; i < 64; i++) {  // 进行足够次数的二分
      double mid = (left + right) / 2;
      if (check(mid)) {
        left = mid;
      } else {
        right = mid;
      }
    }
    cout << static_cast<int>(100 * (left + 0.005)) << "\n";  // 四舍五入
  }
  return 0;
}
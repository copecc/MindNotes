#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 public:
  int maxProfit(vector<int>& prices) {
    int min_price = prices[0];
    int ans = 0;
    for (int price : prices) {
      min_price = min(min_price, price);
      ans = max(ans, price - min_price);
    }
    return ans;
  }
};

#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 public:
  int maxArea(vector<int>& height) {
    int left = 0, right = static_cast<int>(height.size()) - 1;
    int ans = 0;
    while (left < right) {
      ans = max(ans, min(height[left], height[right]) * (right - left));
      if (height[left] < height[right]) {
        ++left;
      } else {
        --right;
      }
    }
    return ans;
  }
};

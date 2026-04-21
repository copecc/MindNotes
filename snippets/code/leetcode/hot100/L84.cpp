#include <algorithm>
#include <stack>
#include <vector>
using namespace std;

class Solution {
 public:
  int largestRectangleArea(vector<int> &heights) {
    heights.push_back(0);
    stack<int> st;
    int ans = 0;
    for (int i = 0; i < heights.size(); ++i) {
      while (!st.empty() && heights[i] < heights[st.top()]) {
        int h = heights[st.top()];
        st.pop();
        int left = st.empty() ? -1 : st.top();
        ans = max(ans, h * (i - left - 1));
      }
      st.push(i);
    }
    heights.pop_back();
    return ans;
  }
};

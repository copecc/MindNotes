#include <algorithm>
#include <stack>
#include <vector>
using namespace std;

class Solution {
 public:
  int largestRectangleArea(vector<int>& heights) {
    int n = heights.size();
    vector<int> left(n, -1), right(n, n);
    stack<int> st;
    for (int i = 0; i < n; ++i) {
      while (!st.empty() && heights[st.top()] >= heights[i]) {
        st.pop();
      }
      if (!st.empty()) {
        left[i] = st.top();
      }
      st.push(i);
    }
    while (!st.empty()) {
      st.pop();
    }
    for (int i = n - 1; i >= 0; --i) {
      while (!st.empty() && heights[st.top()] >= heights[i]) {
        st.pop();
      }
      if (!st.empty()) {
        right[i] = st.top();
      }
      st.push(i);
    }
    int ans = 0;
    for (int i = 0; i < n; ++i) {
      ans = max(ans, heights[i] * (right[i] - left[i] - 1));  // 左右更小元素之间就是可延展宽度。
    }
    return ans;
  }
};


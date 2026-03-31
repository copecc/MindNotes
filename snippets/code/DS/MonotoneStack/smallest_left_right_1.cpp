#include <iostream>
#include <stack>
#include <vector>
using namespace std;

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int n;
  cin >> n;
  vector<int64_t> nums(n);
  for (int i = 0; i < n; ++i) { cin >> nums[i]; }
  int64_t ans = 0;
  vector<int> left(n, -1);
  vector<int> right(n, -1);
  stack<int> st;  // 单调栈，栈顶元素是最小的
  for (int i = 0; i < n; ++i) {
    // 记录右边满足条件的第一个位置
    while (!st.empty() && nums[i] < nums[st.top()]) {
      right[st.top()] = i;
      st.pop();
    }
    // 记录左边满足条件的最后一个位置
    if (!st.empty()) { left[i] = st.top(); }
    st.push(i);
  }
  // 主循环后修正 left
  for (int i = 0; i < n; ++i) {
    int p = left[i];
    // 修正 left，跳过所有相等元素，保证 left[i] 和 nums[i] 是严格单调的
    while (p != -1 && nums[p] == nums[i]) { p = left[p]; }
    left[i] = p;
  }
  for (int i = 0; i < n; ++i) { cout << left[i] << " " << right[i] << "\n"; }
}
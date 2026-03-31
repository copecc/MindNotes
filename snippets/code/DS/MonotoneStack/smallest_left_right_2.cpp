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
  stack<int> st1;  // 处理左边界
  stack<int> st2;  // 处理右边界
  for (int i = 0; i < n; ++i) {
    // 处理右边界: 栈中元素非减序, 即可能存在相等元素
    while (!st2.empty() && nums[i] < nums[st2.top()]) {
      right[st2.top()] = i;
      st2.pop();
    }
    st2.push(i);
    // 处理左边界: 栈中元素严格单调递增
    while (!st1.empty() && nums[i] <= nums[st1.top()]) { st1.pop(); }
    if (!st1.empty()) { left[i] = st1.top(); }
    st1.push(i);
  }
  for (int i = 0; i < n; ++i) { cout << left[i] << " " << right[i] << "\n"; }
}
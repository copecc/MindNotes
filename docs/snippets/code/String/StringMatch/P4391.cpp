#include <iostream>
#include <vector>
using namespace std;

vector<int> get_next(const string &pattern) {
  vector<int> next(pattern.length());
  // j 表示当前匹配的前缀长度
  for (int i = 1, j = 0; i < pattern.length(); ++i) {
    while (j > 0 && pattern[i] != pattern[j]) { j = next[j - 1]; }
    if (pattern[i] == pattern[j]) { ++j; }
    next[i] = j;
  }
  return next;
}

int main() {
  int n;
  cin >> n;
  string pattern;
  cin >> pattern;
  vector<int> next = get_next(pattern);
  cout << n - next[n - 1] << '\n';
  return 0;
}
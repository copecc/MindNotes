#include <cctype>
#include <stack>
#include <string>
using namespace std;

class Solution {
 public:
  string decodeString(string s) {
    stack<int> cnt;
    stack<string> str;
    string cur;
    int num = 0;
    for (char c : s) {
      if (isdigit(c)) {
        num = num * 10 + (c - '0');
      } else if (c == '[') {
        cnt.push(num);
        str.push(cur);
        num = 0;
        cur.clear();
      } else if (c == ']') {
        string prev = str.top();
        str.pop();
        int repeat = cnt.top();
        cnt.pop();
        while (repeat--) {
          prev += cur;
        }
        cur = prev;
      } else {
        cur.push_back(c);
      }
    }
    return cur;
  }
};

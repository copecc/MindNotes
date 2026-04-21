#include <algorithm>
#include <string>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<int> partitionLabels(string s) {
    int n = s.size();
    vector<int> last(26, 0);
    for (int i = 0; i < n; ++i) {
      last[s[i] - 'a'] = i;
    }

    vector<int> ans;
    int start = 0;
    int end = 0;
    for (int i = 0; i < n; ++i) {
      end = max(end, last[s[i] - 'a']);
      if (i == end) {
        ans.push_back(i - start + 1);
        start = i + 1;
      }
    }
    return ans;
  }
};

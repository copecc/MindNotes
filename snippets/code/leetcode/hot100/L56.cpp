#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<vector<int>> merge(vector<vector<int>> &intervals) {
    sort(intervals.begin(), intervals.end());
    vector<vector<int>> ans;
    for (const auto &cur : intervals) {
      if (ans.empty() || ans.back()[1] < cur[0]) {
        ans.push_back(cur);
      } else {
        ans.back()[1] = max(ans.back()[1], cur[1]);
      }
    }
    return ans;
  }
};


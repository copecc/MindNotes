#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 private:
  void dfs(int start, int target, vector<int> &nums, vector<int> &path, vector<vector<int>> &ans) {
    if (target == 0) {
      ans.push_back(path);
      return;
    }
    for (int i = start; i < nums.size(); ++i) {
      if (nums[i] > target) break;
      path.push_back(nums[i]);
      dfs(i, target - nums[i], nums, path, ans);
      path.pop_back();
    }
  }

 public:
  vector<vector<int>> combinationSum(vector<int> &candidates, int target) {
    sort(candidates.begin(), candidates.end());
    vector<vector<int>> ans;
    vector<int> path;
    dfs(0, target, candidates, path, ans);
    return ans;
  }
};

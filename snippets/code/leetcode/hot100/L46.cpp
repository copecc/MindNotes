#include <algorithm>
#include <vector>
using namespace std;

class Solution {
 private:
  void dfs(int start, vector<int> &nums, vector<vector<int>> &ans) {
    if (start == nums.size()) {
      ans.push_back(nums);
      return;
    }
    for (int i = start; i < nums.size(); ++i) {
      swap(nums[start], nums[i]);
      dfs(start + 1, nums, ans);
      swap(nums[start], nums[i]);
    }
  }

 public:
  vector<vector<int>> permute(vector<int> &nums) {
    vector<vector<int>> ans;
    dfs(0, nums, ans);
    return ans;
  }
};

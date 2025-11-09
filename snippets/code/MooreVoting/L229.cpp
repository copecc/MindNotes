#include <vector>
using namespace std;

class Solution {
 public:
  vector<int> majorityElement(vector<int> &nums) {
    int k = 3;  // Majority threshold is n/k
    vector<pair<int, int>> candidates(k - 1, {0, 0});
    for (int num : nums) { UpdateMajority(candidates, num); }
    vector<int> result;
    result.reserve(candidates.size());
    for (auto [candidate, count] : candidates) {  // verify candidates
      if (count == 0) { continue; }
      count = 0;
      for (int num : nums) {
        if (num == candidate) { count++; }
      }
      if (count > nums.size() / k) { result.push_back(candidate); }
    }
    return result;
  }

  void UpdateMajority(vector<pair<int, int>> &candidates, int num) {
    for (auto &[candidate, count] : candidates) {  // Already a candidate
      if (candidate == num && count > 0) {
        count++;
        return;
      }
    }
    for (auto &[candidate, count] : candidates) {  // Find an empty candidate slot
      if (count == 0) {
        candidate = num;
        count     = 1;
        return;
      }
    }
    // Decrease count for all candidates
    for (auto &[candidate, count] : candidates) { count = max(0, count - 1); }
  }
};
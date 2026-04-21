#include <functional>
#include <queue>
#include <unordered_map>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<int> topKFrequent(vector<int> &nums, int k) {
    unordered_map<int, int> cnt;
    for (int x : nums) {
      ++cnt[x];
    }

    using Node = pair<int, int>;
    priority_queue<Node, vector<Node>, greater<Node>> pq;
    for (auto &[num, freq] : cnt) {
      pq.push({freq, num});
      if (pq.size() > k) {
        pq.pop();
      }
    }

    vector<int> ans;
    while (!pq.empty()) {
      ans.push_back(pq.top().second);
      pq.pop();
    }
    return ans;
  }
};

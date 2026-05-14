#include <queue>
#include <vector>
using namespace std;

class Solution {
 public:
  int coinChange(vector<int>& coins, int amount) {
    if (amount == 0) {
      return 0;
    }
    vector<bool> visited(amount + 1, false);
    queue<int> q;
    q.push(0);
    visited[0] = true;
    int steps = 0;
    while (!q.empty()) {
      ++steps;
      int size = q.size();
      while (size--) {
        int cur = q.front();
        q.pop();
        for (int coin : coins) {
          int next = cur + coin;
          if (next == amount) {
            return steps;
          }
          if (next > amount || visited[next]) {
            continue;
          }
          visited[next] = true;  // 同一金额只入队一次，避免重复扩展。
          q.push(next);
        }
      }
    }
    return -1;
  }
};

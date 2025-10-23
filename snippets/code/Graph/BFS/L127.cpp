#include <unordered_set>
#include <vector>
using namespace std;

class Solution {
 public:
  int ladderLength(string beginWord, string endWord, vector<string> &wordList) {
    unordered_set<string> wordSet(wordList.begin(), wordList.end());
    if (wordSet.find(endWord) == wordSet.end()) { return 0; }

    unordered_set<string> small{beginWord};  // 从beginWord开始搜索（小集合）
    unordered_set<string> large{endWord};    // 从endWord开始搜索（大集合）
    unordered_set<string> next;              // 下一层节点

    int step = 1;
    while (!small.empty() && !large.empty()) {
      if (small.size() > large.size()) { swap(small, large); }  // 保持small是较小的集合
      for (const string &word : small) { wordSet.erase(word); }  // 移除已访问的单词
      for (const string &word : small) {
        string curr = word;
        // 枚举所有可能的单词
        for (int i = 0; i < curr.size(); ++i) {
          char original = curr[i];
          for (char c = 'a'; c <= 'z'; ++c) {
            if (c == original) { continue; }
            curr[i] = c;
            if (large.find(curr) != large.end()) { return step + 1; }
            if (wordSet.find(curr) != wordSet.end()) { next.insert(curr); }
          }
          curr[i] = original;
        }
      }

      swap(small, next);
      next.clear();
      ++step;
    }
    return 0;
  }
};
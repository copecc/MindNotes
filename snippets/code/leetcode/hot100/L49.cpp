#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

class Solution {
 public:
  vector<vector<string>> groupAnagrams(vector<string> &strs) {
    unordered_map<string, vector<string>> groups;
    for (const string &s : strs) {
      string key = s;
      // 排序后的字符串可以唯一表示同一组异位词。
      sort(key.begin(), key.end());
      groups[key].push_back(s);
    }

    vector<vector<string>> ans;
    for (auto &entry : groups) {
      ans.push_back(std::move(entry.second));
    }
    return ans;
  }
};

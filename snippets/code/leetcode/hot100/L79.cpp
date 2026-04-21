#include <string>
#include <vector>
using namespace std;

class Solution {
 private:
  bool dfs(vector<vector<char>> &board, const string &word, int idx, int x, int y) {
    if (idx == word.size()) return true;
    int m = board.size();
    int n = board[0].size();
    if (x < 0 || x >= m || y < 0 || y >= n || board[x][y] != word[idx]) return false;

    char saved = board[x][y];
    board[x][y] = '#';
    bool found = dfs(board, word, idx + 1, x + 1, y) ||
                 dfs(board, word, idx + 1, x - 1, y) ||
                 dfs(board, word, idx + 1, x, y + 1) ||
                 dfs(board, word, idx + 1, x, y - 1);
    board[x][y] = saved;
    return found;
  }

 public:
  bool exist(vector<vector<char>> &board, string word) {
    int m = board.size();
    int n = board[0].size();
    for (int i = 0; i < m; ++i) {
      for (int j = 0; j < n; ++j) {
        if (dfs(board, word, 0, i, j)) return true;
      }
    }
    return false;
  }
};

#include <string>
#include <vector>
using namespace std;

class Solution {
 private:
  void dfs(int row, int n, vector<int> &pos, vector<bool> &col, vector<bool> &diag1,
           vector<bool> &diag2, vector<vector<string>> &ans) {
    if (row == n) {
      vector<string> board(n, string(n, '.'));
      for (int i = 0; i < n; ++i) board[i][pos[i]] = 'Q';
      ans.push_back(board);
      return;
    }
    for (int c = 0; c < n; ++c) {
      int d1 = row + c;
      int d2 = row - c + n - 1;
      if (col[c] || diag1[d1] || diag2[d2]) continue;
      pos[row] = c;
      col[c] = diag1[d1] = diag2[d2] = true;
      dfs(row + 1, n, pos, col, diag1, diag2, ans);
      col[c] = diag1[d1] = diag2[d2] = false;
    }
  }

 public:
  vector<vector<string>> solveNQueens(int n) {
    vector<vector<string>> ans;
    vector<int> pos(n, -1);
    vector<bool> col(n, false), diag1(2 * n - 1, false), diag2(2 * n - 1, false);
    dfs(0, n, pos, col, diag1, diag2, ans);
    return ans;
  }
};

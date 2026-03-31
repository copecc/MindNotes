#include <iostream>
#include <vector>
using namespace std;

bool misere_nim_game(vector<int> &piles) {
  int xor_sum    = 0;
  int count_ones = 0;  // 只有一颗石头的堆数
  for (int stones : piles) {
    xor_sum ^= stones;
    if (stones == 1) { count_ones++; }
  }
  if (count_ones == piles.size()) {  // 全是单石堆
    return (count_ones % 2 == 0);    // 石头堆数为偶数时，先手必胜
  }
  return (xor_sum != 0);  // 否则与普通尼姆博弈相同
}

void solve() {
  int n;
  cin >> n;
  vector<int> piles(n);
  for (int i = 0; i < n; i++) { cin >> piles[i]; }
  if (misere_nim_game(piles)) {
    cout << "John\n";
  } else {
    cout << "Brother\n";
  }
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int64_t t = 1;
  cin >> t;
  while ((t--) != 0) { solve(); }
  return 0;
}
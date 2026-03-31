#include <iostream>
#include <vector>
using namespace std;

bool nim_game(const vector<int> &piles) {
  int xor_sum = 0;
  for (int stones : piles) { xor_sum ^= stones; }
  return (xor_sum != 0);
}

void solve() {
  int n;
  cin >> n;
  vector<int> piles(n);
  for (int i = 0; i < n; i++) { cin >> piles[i]; }
  if (nim_game(piles)) {
    cout << "Yes\n";
  } else {
    cout << "No\n";
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
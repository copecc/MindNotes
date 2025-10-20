#include <iostream>
using namespace std;

int main() {
  int t, n;
  cin >> t;
  for (int i = 1; i <= t; i++) {
    cin >> n;
    if (n % 6 == 0)
      cout << "Roy wins!" << endl;
    else
      cout << "October wins!" << endl;
  }
}
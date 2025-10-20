#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

using Pdd = pair<double, double>;

Pdd sub(const Pdd &a, const Pdd &b) { return {a.first - b.first, a.second - b.second}; }

double dist(const Pdd &a, const Pdd &b) {
  Pdd d = sub(a, b);
  return sqrt(d.first * d.first + d.second * d.second);
}

Pdd internal_division(const Pdd &a, const Pdd &b,
                      double p) {  // a + (b - a) * p
  return {a.first + (b.first - a.first) * p, a.second + (b.second - a.second) * p};
}

const int NUM_ITERATION = 60;

// 计算线段 AB 与原点最短距离
double dist_segment_and_origin(const Pdd &a, const Pdd &b) {
  auto f   = [&](double t) { return dist(internal_division(a, b, t), {0.0, 0.0}); };
  double l = 0.0, r = 1.0;
  for (int i = 0; i < NUM_ITERATION; ++i) {
    double m1 = (2 * l + r) / 3.0;
    double m2 = (l + 2 * r) / 3.0;
    if (f(m1) < f(m2)) {  // 极小值在左侧
      r = m2;
    } else {  // 极小值在右侧
      l = m1;
    }
  }
  return f((l + r) / 2.0);
}

void solve() {
  vector<Pdd> s(2), g(2);
  cin >> s[0].first >> s[0].second >> g[0].first >> g[0].second;
  cin >> s[1].first >> s[1].second >> g[1].first >> g[1].second;

  // 保证 (s[0], g[0]) 是较长线段
  double len0 = dist(s[0], g[0]);
  double len1 = dist(s[1], g[1]);
  if (len0 < len1) {
    swap(s[0], s[1]);
    swap(g[0], g[1]);
    swap(len0, len1);
  }

  // 长线段按短线段长度比例取中点
  Pdd mid_point = internal_division(s[0], g[0], len1 / len0);

  // 三分法分别计算两段距离
  // (s[0],mid_point) 上的移动点到线段 (s[1],g[1]) 的最小值
  double d1 = dist_segment_and_origin(sub(s[0], s[1]), sub(mid_point, g[1]));
  // (mid_point,g[0]) 上的移动点到最后 (s[1],g[1]) 的最小值
  double d2 = dist_segment_and_origin(sub(mid_point, g[1]), sub(g[0], g[1]));

  cout << fixed << setprecision(15) << min(d1, d2) << "\n";
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  cout.tie(nullptr);
  int t;
  cin >> t;
  while ((t--) != 0) { solve(); }
}
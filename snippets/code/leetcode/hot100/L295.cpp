#include <functional>
#include <queue>
#include <vector>
using namespace std;

class MedianFinder {
 public:
  MedianFinder() = default;

  void addNum(int num) {
    if (small_.empty() || num <= small_.top()) {
      small_.push(num);
    } else {
      large_.push(num);
    }

    if (small_.size() > large_.size() + 1) {
      large_.push(small_.top());
      small_.pop();
    } else if (large_.size() > small_.size()) {
      small_.push(large_.top());
      large_.pop();
    }
  }

  double findMedian() {
    if (small_.size() > large_.size()) {
      return small_.top();
    }
    return (small_.top() + large_.top()) / 2.0;
  }

 private:
  priority_queue<int> small_;
  priority_queue<int, vector<int>, greater<int>> large_;
};

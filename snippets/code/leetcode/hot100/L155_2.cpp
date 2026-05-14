#include <stack>
using namespace std;

class MinStack {
 public:
  MinStack() = default;

  void push(int val) {
    if (st_.empty()) {
      st_.push(0);
      min_ = val;
      return;
    }
    long long diff = static_cast<long long>(val) - min_;
    st_.push(diff);
    if (diff < 0) {
      min_ = val;  // 负差值说明新元素本身就是新的最小值。
    }
  }

  void pop() {
    long long diff = st_.top();
    st_.pop();
    if (diff < 0) {
      min_ -= diff;
    }
  }

  int top() {
    long long diff = st_.top();
    if (diff >= 0) {
      return static_cast<int>(min_ + diff);
    }
    return static_cast<int>(min_);
  }

  int getMin() {
    return static_cast<int>(min_);
  }

 private:
  stack<long long> st_;
  long long min_ = 0;
};

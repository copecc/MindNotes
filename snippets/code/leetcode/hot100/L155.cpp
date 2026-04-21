#include <stack>
using namespace std;

class MinStack {
 public:
  MinStack() = default;

  void push(int val) {
    data_.push(val);
    if (mins_.empty() || val <= mins_.top()) {
      mins_.push(val);
    }
  }

  void pop() {
    if (data_.top() == mins_.top()) {
      mins_.pop();
    }
    data_.pop();
  }

  int top() {
    return data_.top();
  }

  int getMin() {
    return mins_.top();
  }

 private:
  stack<int> data_;
  stack<int> mins_;
};


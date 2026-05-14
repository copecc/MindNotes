#include <vector>
using namespace std;

class Solution {
 public:
  ListNode *mergeKLists(vector<ListNode *> &lists) {
    return merge(lists, 0, lists.size());
  }

 private:
  ListNode *merge(vector<ListNode *> &lists, int left, int right) {
    if (left >= right) return nullptr;
    if (left + 1 == right) return lists[left];
    int mid = left + (right - left) / 2;
    return mergeTwoLists(merge(lists, left, mid), merge(lists, mid, right));  // 把 k 路归并不断降回两路归并。
  }

  ListNode *mergeTwoLists(ListNode *a, ListNode *b) {
    ListNode dummy(0);
    ListNode *tail = &dummy;
    while (a && b) {
      if (a->val < b->val) {
        tail->next = a;
        a = a->next;
      } else {
        tail->next = b;
        b = b->next;
      }
      tail = tail->next;
    }
    tail->next = a ? a : b;
    return dummy.next;
  }
};

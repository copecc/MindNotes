#include <functional>
#include <queue>
#include <vector>
using namespace std;

class Solution {
 public:
  ListNode *mergeKLists(vector<ListNode *> &lists) {
    auto cmp = [](ListNode *a, ListNode *b) { return a->val > b->val; };
    priority_queue<ListNode *, vector<ListNode *>, decltype(cmp)> pq(cmp);
    for (ListNode *head : lists) {
      if (head) pq.push(head);
    }

    ListNode dummy(0);
    ListNode *tail = &dummy;
    while (!pq.empty()) {
      ListNode *node = pq.top();
      pq.pop();
      if (node->next) pq.push(node->next);
      tail->next = node;
      tail = tail->next;
    }
    return dummy.next;
  }
};

#include <algorithm>
#include <stack>
#include <vector>
using namespace std;

class Solution {
 public:
  ListNode *reverseKGroup(ListNode *head, int k) {
    ListNode dummy(0, head);
    ListNode *groupPrev = &dummy;
    while (true) {
      ListNode *kth = groupPrev;
      for (int i = 0; i < k && kth; ++i) {
        kth = kth->next;
      }
      if (!kth) break;
      ListNode *groupNext = kth->next;
      ListNode *prev = groupNext;
      ListNode *cur = groupPrev->next;
      while (cur != groupNext) {
        ListNode *next = cur->next;
        cur->next = prev;  // 当前组内原地翻转，尾指针先连到下一组头部。
        prev = cur;
        cur = next;
      }
      ListNode *newGroupTail = groupPrev->next;
      groupPrev->next = kth;
      groupPrev = newGroupTail;
    }
    return dummy.next;
  }
};


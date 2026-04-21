class Solution {
 public:
  ListNode *removeNthFromEnd(ListNode *head, int n) {
    ListNode dummy(0, head);
    ListNode *fast = &dummy;
    ListNode *slow = &dummy;
    while (n--) {
      fast = fast->next;
    }
    while (fast->next) {
      fast = fast->next;
      slow = slow->next;
    }
    ListNode *del = slow->next;
    slow->next = del->next;
    return dummy.next;
  }
};

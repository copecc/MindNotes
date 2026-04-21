class Solution {
 public:
  ListNode *reverseKGroup(ListNode *head, int k) {
    ListNode *kth = head;
    for (int i = 0; i < k; ++i) {
      if (!kth) return head;
      kth = kth->next;
    }

    ListNode *groupNext = kth;
    ListNode *prev = groupNext;
    ListNode *cur = head;
    while (cur != groupNext) {
      ListNode *next = cur->next;
      cur->next = prev;
      prev = cur;
      cur = next;
    }

    head->next = reverseKGroup(groupNext, k);
    return prev;
  }
};

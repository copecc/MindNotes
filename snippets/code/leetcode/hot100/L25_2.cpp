class Solution {
 public:
  ListNode *reverseKGroup(ListNode *head, int k) {
    ListNode *tail = head;
    for (int i = 0; i < k; ++i) {
      if (!tail) return head;
      tail = tail->next;
    }
    ListNode *newHead = reverseRange(head, tail);
    head->next = reverseKGroup(tail, k);
    return newHead;
  }

 private:
  ListNode *reverseRange(ListNode *head, ListNode *tail) {
    ListNode *prev = tail;
    while (head != tail) {
      ListNode *next = head->next;
      head->next = prev;
      prev = head;
      head = next;
    }
    return prev;
  }
};

class Solution {
 public:
  ListNode *reverseBetween(ListNode *head, int left, int right) {
    if (!head || left == right) {
      return head;
    }
    ListNode dummy(0, head);
    ListNode *prev = &dummy;
    for (int i = 0; i < left - 1; ++i) {
      prev = prev->next;
    }
    ListNode *cur = prev->next;
    for (int i = 0; i < right - left; ++i) {
      ListNode *next = cur->next;
      cur->next = next->next;
      next->next = prev->next;  // 头插到区间前端。
      prev->next = next;
    }
    return dummy.next;
  }
};


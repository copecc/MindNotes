class Solution {
 public:
  ListNode *reverseList(ListNode *head) {
    ListNode *prev = nullptr;
    ListNode *cur = head;
    while (cur) {
      ListNode *next = cur->next;
      cur->next = prev;  // 当前节点改指向已反转前缀，再整体前移。
      prev = cur;
      cur = next;
    }
    return prev;
  }
};

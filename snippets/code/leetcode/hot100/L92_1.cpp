class Solution {
 public:
  ListNode *reverseBetween(ListNode *head, int left, int right) {
    if (!head || left == right) {
      return head;
    }
    if (left == 1) {
      return reverseFrontN(head, right);
    }
    head->next = reverseBetween(head->next, left - 1, right - 1);
    return head;
  }

 private:
  ListNode *successor_ = nullptr;

  ListNode *reverseFrontN(ListNode *head, int n) {
    if (n == 1) {
      successor_ = head->next;  // 记录反转区间后的第一个节点，回溯时接回去。
      return head;
    }
    ListNode *newHead = reverseFrontN(head->next, n - 1);
    head->next->next = head;
    head->next = successor_;
    return newHead;
  }
};


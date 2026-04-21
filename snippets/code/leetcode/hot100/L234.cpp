class Solution {
 private:
  ListNode *reverseList(ListNode *head) {
    ListNode *prev = nullptr;
    while (head) {
      ListNode *next = head->next;
      head->next = prev;
      prev = head;
      head = next;
    }
    return prev;
  }

 public:
  bool isPalindrome(ListNode *head) {
    if (!head || !head->next) return true;

    ListNode *slow = head;
    ListNode *fast = head;
    while (fast->next && fast->next->next) {
      slow = slow->next;
      fast = fast->next->next;
    }

    ListNode *second = reverseList(slow->next);
    slow->next = nullptr;

    ListNode *p1 = head;
    ListNode *p2 = second;
    bool ok = true;
    while (p2) {
      if (p1->val != p2->val) {
        ok = false;
        break;
      }
      p1 = p1->next;
      p2 = p2->next;
    }

    slow->next = reverseList(second);
    return ok;
  }
};

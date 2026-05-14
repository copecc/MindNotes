class Solution {
 public:
  Node *copyRandomList(Node *head) {
    if (!head) return nullptr;

    for (Node *cur = head; cur; cur = cur->next->next) {
      Node *copy = new Node(cur->val);
      copy->next = cur->next;
      cur->next = copy;
    }

    for (Node *cur = head; cur; cur = cur->next->next) {
      if (cur->random) cur->next->random = cur->random->next;
    }

    Node *newHead = head->next;
    for (Node *cur = head; cur; cur = cur->next) {
      Node *copy = cur->next;
      cur->next = copy->next;
      if (copy->next) copy->next = copy->next->next;  // 拆分旧链表时，同步跳过下一份拷贝。
    }
    return newHead;
  }
};

#include <unordered_map>
using namespace std;

class Solution {
 public:
  Node *copyRandomList(Node *head) {
    if (!head) return nullptr;

    unordered_map<Node *, Node *> copy;
    for (Node *cur = head; cur; cur = cur->next) {
      copy[cur] = new Node(cur->val);
    }

    for (Node *cur = head; cur; cur = cur->next) {
      copy[cur]->next = cur->next ? copy[cur->next] : nullptr;
      copy[cur]->random = cur->random ? copy[cur->random] : nullptr;
    }

    return copy[head];
  }
};

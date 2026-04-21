#include <string>
#include <vector>
using namespace std;

class Trie {
 private:
  struct Node {
    Node *child[26];
    bool end;

    Node() : end(false) {
      for (int i = 0; i < 26; ++i) child[i] = nullptr;
    }
  };

  Node *root;

  Node *findNode(const string &word) {
    Node *cur = root;
    for (char ch : word) {
      int idx = ch - 'a';
      if (!cur->child[idx]) return nullptr;
      cur = cur->child[idx];
    }
    return cur;
  }

 public:
  Trie() : root(new Node()) {
  }

  void insert(string word) {
    Node *cur = root;
    for (char ch : word) {
      int idx = ch - 'a';
      if (!cur->child[idx]) cur->child[idx] = new Node();
      cur = cur->child[idx];
    }
    cur->end = true;
  }

  bool search(string word) {
    Node *node = findNode(word);
    return node && node->end;
  }

  bool startsWith(string prefix) {
    return findNode(prefix) != nullptr;
  }
};

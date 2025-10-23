#include <vector>
using namespace std;

class Solution {
  struct Trie {
    Trie() = default;

    struct TrieNode {
      TrieNode *left  = nullptr;  // 0
      TrieNode *right = nullptr;  // 1
    };

    const int L    = 30;  // 31位整数，最高位符号位不考虑
    TrieNode *root = new TrieNode();

    void Insert(int num) {
      TrieNode *node = root;
      for (int i = L; i >= 0; i--) {
        int bit = (num >> i) & 1;
        if (bit == 0) {
          if (!node->left) { node->left = new TrieNode(); }
          node = node->left;
        } else {
          if (!node->right) { node->right = new TrieNode(); }
          node = node->right;
        }
      }
    }

    int GetMaxXor(int num) {
      TrieNode *node = root;
      int maxXor     = 0;
      for (int i = L; i >= 0; i--) {
        int bit = (num >> i) & 1;
        if (bit == 0) {
          if (node->right) {  // 有1
            node    = node->right;
            maxXor |= (1 << i);
          } else {  // 没有1
            node = node->left;
          }
        } else {             // bit == 1
          if (node->left) {  // 有0
            node    = node->left;
            maxXor |= (1 << i);
          } else {  // 没有0
            node = node->right;
          }
        }
      }
      return maxXor;
    }
  };

 public:
  int findMaximumXOR(vector<int> &nums) {
    Trie trie;
    for (int num : nums) { trie.Insert(num); }

    int maxXor = 0;
    for (int num : nums) { maxXor = max(maxXor, trie.GetMaxXor(num)); }
    return maxXor;
  }
};
---
title: 字典树
tags:
  - Trie
  - 前缀树
  - 字典树
  - 0-1字典树
---

# 字典树

字典树（$\text{Trie}$），又称前缀树或单词查找树，是一种用于高效存储和检索字符串集合的数据结构。它的主要特点是通过公共前缀来节省存储空间，并支持快速的字符串查找操作。

???+ note "Trie"

    === "动态内存实现"

        ```cpp
        struct Trie {
          Trie() = default;

          struct TrieNode {
            int pass = 0;  // 经过该节点的字符串数量
            int end  = 0;  // 以该节点结尾的字符串数量
            std::unordered_map<char, std::unique_ptr<TrieNode>> children;
          };

          std::unique_ptr<TrieNode> root = std::make_unique<TrieNode>();

          void Insert(const string &word) {
            TrieNode *node = root.get();
            node->pass++;
            for (char ch : word) {
              if (!node->children.contains(ch)) { node->children[ch] = std::make_unique<TrieNode>(); }
              node = node->children[ch].get();
              node->pass++;
            }
            node->end++;
          }

          int CountWordsEqualTo(const string &word) {
            TrieNode *node = root.get();
            for (char ch : word) {
              if (!node->children.contains(ch)) { return 0; }
              node = node->children[ch].get();
            }
            return node->end;
          }

          int CountWordsStartingWith(const string &prefix) {
            TrieNode *node = root.get();
            for (char ch : prefix) {
              if (!node->children.contains(ch)) { return 0; }
              node = node->children[ch].get();
            }
            return node->pass;
          }

          void Erase(const string &word) {
            if (CountWordsEqualTo(word) == 0) { return; }  // 单词不存在，无法删除
            TrieNode *node = root.get();
            node->pass--;
            for (char ch : word) {
              TrieNode *next_node = node->children[ch].get();
              next_node->pass--;
              // 如果经过该节点的字符串数量为0，说明该节点不再需要，删除该节点及其子树
              if (next_node->pass == 0) {
                node->children.erase(ch);
                return;
              }
              node = next_node;
            }
            node->end--;
          }
        };
        ```

    === "静态内存实现"

        ```cpp
        struct Trie {
          const static int max_nodes = 100'000;  // 最大节点数
          // tree[i][j]表示节点i的第j个子节点
          inline static vector<array<int, 26>> tree = vector<array<int, 26>>(max_nodes);
          // 经过该节点的字符串数量
          inline static vector<int> pass = vector<int>(max_nodes);
          // 以该节点结尾的字符串数量
          inline static vector<int> end = vector<int>(max_nodes);

          int count                     = 1;  // 当前节点总数，根节点为1

        public:
          Trie() = default;

          void Insert(const string &word) {
            int current = 1;
            pass[current]++;
            for (char ch : word) {
              int index = ch - 'a';
              if (tree[current][index] == 0) { tree[current][index] = ++count; }
              current = tree[current][index];
              pass[current]++;
            }
            end[current]++;
          }

          int CountWordsEqualTo(const string &word) {
            int current = 1;
            for (char ch : word) {
              int index = ch - 'a';
              if (tree[current][index] == 0) { return 0; }
              current = tree[current][index];
            }
            return end[current];
          }

          int CountWordsStartingWith(const string &prefix) {
            int current = 1;
            for (char ch : prefix) {
              int index = ch - 'a';
              if (tree[current][index] == 0) { return 0; }
              current = tree[current][index];
            }
            return pass[current];
          }

          void Erase(const string &word) {
            if (CountWordsEqualTo(word) == 0) { return; }  // 单词不存在，无法删除
            int current = 1;
            pass[current]--;
            for (char ch : word) {
              int index     = ch - 'a';
              int next_node = tree[current][index];
              pass[next_node]--;
              // 如果经过该节点的字符串数量为0，说明该节点不再需要，删除该节点及其子树
              if (pass[next_node] == 0) {
                tree[current][index] = 0;
                return;
              }
              current = next_node;
            }
            end[current]--;
          }

          // 重置Trie树, 每次调用后相当于新建一个Trie树
          void Clear() {
            count = 1;
            std::fill(pass.begin(), pass.end(), 0);
            std::fill(end.begin(), end.end(), 0);
            for (auto &child : tree) { std::fill(child.begin(), child.end(), 0); }
          }
        };
        ```

!!! tip "处理数字的技巧"

    将数字转换成字符，然后每个数字结尾加一个特殊字符，例如 $'\#'$ 表示一整个数字的结束。例如数字 $-123$，转换成字符串 $'-123\#'$，这样就不用增加 $tree$ 第二维的大小，使用 $12$ 个字符$('0'-'9', '-', '\#')$就能表示所有数字。

??? note "[数组中两个数的最大异或值](https://leetcode.cn/problems/maximum-xor-of-two-numbers-in-an-array/description/){target=_blank}"

    !!! tip "$\text{0-1}$字典树"

    ```cpp
        --8<-- "code/String/Trie/L421.cpp"
    ```
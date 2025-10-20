#include <iostream>
#include <queue>
#include <unordered_map>

using namespace std;

struct Node {
  int val;
  vector<Node *> children;

  Node(int v) : val(v) {}
};

Node *build_tree(const vector<int> &pre_order, const vector<int> &post_order) {
  int n = pre_order.size();
  unordered_map<int, int> pos;
  for (int i = 0; i < n; ++i) { pos[post_order[i]] = i; }  // 后序位置索引

  int root_index = 0;  // 当前处理的前序下标
  // 左闭右闭区间
  function<Node *(int, int)> build = [&](int left, int right) -> Node * {
    if (left > right) { return nullptr; }
    Node *root = new Node(pre_order[root_index]);
    root_index++;
    if (left == right) { return root; }

    while (root_index < n && left <= right - 1) {
      int child_val = pre_order[root_index];
      int j         = pos[child_val];
      if (j > right - 1) break;
      root->children.push_back(build(left, j));
      left = j + 1;
    }
    return root;
  };

  return build(0, n - 1);
}

vector<int> level_order(Node *root) {
  if (root == nullptr) return {};
  // 层序遍历找每层最右节点
  queue<pair<Node *, int>> q;
  q.push({root, 0});
  vector<int> rightmost;

  while (!q.empty()) {
    auto [node, depth] = q.front();
    q.pop();
    if ((int)rightmost.size() <= depth)
      rightmost.push_back(node->val);
    else
      rightmost[depth] = node->val;
    for (auto ch : node->children) q.push({ch, depth + 1});
  }
  return rightmost;
}

int main() {
  int n;
  {
    n                      = 7;
    vector<int> pre_order  = {1, 2, 4, 5, 3, 6, 7};
    vector<int> post_order = {4, 5, 2, 6, 7, 3, 1};

    Node *root             = build_tree(pre_order, post_order);
    vector<int> rightmost  = level_order(root);
    // 1 3 7
    for (int x : rightmost) { cout << x << " "; };
    cout << "\n";
  }
  {
    cin >> n;
    vector<int> pre_order(n), post_order(n);
    for (int i = 0; i < n; ++i) { cin >> pre_order[i]; }
    for (int i = 0; i < n; ++i) { cin >> post_order[i]; }

    Node *root            = build_tree(pre_order, post_order);
    vector<int> rightmost = level_order(root);
    for (int x : rightmost) { cout << x << " "; };
    cout << "\n";
  }
  return 0;
}
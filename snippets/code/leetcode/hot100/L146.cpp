#include <list>
#include <unordered_map>
#include <utility>
using namespace std;

class LRUCache {
 private:
  int capacity;
  list<pair<int, int>> cache;
  unordered_map<int, list<pair<int, int>>::iterator> pos;

 public:
  LRUCache(int capacity) : capacity(capacity) {
  }

  int get(int key) {
    auto it = pos.find(key);
    if (it == pos.end()) return -1;
    cache.splice(cache.begin(), cache, it->second);
    return it->second->second;
  }

  void put(int key, int value) {
    auto it = pos.find(key);
    if (it != pos.end()) {
      it->second->second = value;
      cache.splice(cache.begin(), cache, it->second);
      return;
    }

    if (cache.size() == static_cast<size_t>(capacity)) {
      pos.erase(cache.back().first);
      cache.pop_back();
    }

    cache.emplace_front(key, value);
    pos[key] = cache.begin();
  }
};

module;

#include <cstddef>
#include <functional>
#include <list>
#include <unordered_map>

export module lru_cache;

import base_cache;

namespace caches {

export template <typename Key, typename Value>
class LRUCache final : public BaseCache<Key, Value> {

public:
  explicit LRUCache(std::size_t capacity) : BaseCache<Key, Value>(capacity) {}

private:
  using BaseCache<Key, Value>::max_capacity;

  // Each entry is {key, page}; most recently used entry is at the front.
  std::list<std::pair<Key, Value>> cache_;

  using ListIt = typename std::list<std::pair<Key, Value>>::iterator;
  std::unordered_map<Key, ListIt> hash_;

  bool is_full() const { return (cache_.size() == max_capacity()); }

  bool do_lookup_update(const Key& key, std::function<Value(Key)> slow_get_page) override {
    auto hit = hash_.find(key);
    if (hit != hash_.end()) {
      auto eltit = hit->second;
      cache_.splice(cache_.begin(), cache_, eltit);
      return true;
    }

    auto page = slow_get_page(key);

    if (is_full()) {
      hash_.erase(cache_.back().first);
      cache_.pop_back();
    }
    cache_.emplace_front(key, page);
    hash_.emplace(key, cache_.begin());
    return false;
  }
};

}  // namespace caches

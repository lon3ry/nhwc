module;

#include <algorithm>
#include <cstddef>
#include <functional>

export module arc_cache;

import base_cache;
import lru_queue;

namespace caches {

export template <typename Key, typename Value>
class ARCCache final : public BaseCache<Key, Value> {
public:
  explicit ARCCache(std::size_t capacity) : BaseCache<Key, Value>(capacity) {}

private:
  using BaseCache<Key, Value>::max_capacity;

  LRUQueue<Key, Value> recents_;            // T1: pages seen only once recently
  LRUQueue<Key, Value> frequenters_;        // T2: pages seen at least twice recently
  GhostLRUQueue<Key> evicted_recents_;      // B1: ghost cache for T1
  GhostLRUQueue<Key> evicted_frequenters_;  // B2: ghost cache for T2

  std::size_t p_ = 0;

  void replace(const Key& key) {
    if (!recents_.empty() && (recents_.size() > p_ || (evicted_frequenters_.has(key) &&
        p_ == recents_.size()))) {
      auto victim = recents_.pop_last_recently_used();
      evicted_recents_.insert(victim->key);
    } else {
      auto victim = frequenters_.pop_last_recently_used();
      evicted_frequenters_.insert(victim->key);
    }
  }

  bool do_lookup_update(const Key& key, std::function<Value(Key)> slow_get_page) override {
    bool hit_recents = recents_.lookup(key);
    if (hit_recents) {
      auto item = recents_.pop_most_recently_used();
      frequenters_.insert({item->key, item->page});
      return true;
    }

    bool hit_frequenters = frequenters_.lookup(key);
    if (hit_frequenters) return true;

    bool hit_evicted_recents = evicted_recents_.lookup(key);
    if (hit_evicted_recents) {
      std::size_t delta1 = evicted_recents_.size() >= evicted_frequenters_.size() ?
                           1 :
                           evicted_frequenters_.size() / evicted_recents_.size();
      p_ = std::min(p_ + delta1, max_capacity());

      replace(key);
      evicted_recents_.erase(key);

      auto page = slow_get_page(key);
      frequenters_.insert({key, page});

      return false;
    }

    bool hit_evicted_frequenters = evicted_frequenters_.lookup(key);
    if (hit_evicted_frequenters) {
      std::size_t delta2 = evicted_frequenters_.size() >= evicted_recents_.size() ?
                           1 :
                           evicted_recents_.size() / evicted_frequenters_.size();
      // p_ = std::max<long long>(p_ - delta2, 0);
      p_ = (p_ > delta2) ? (p_ - delta2) : 0;

      replace(key);
      evicted_frequenters_.erase(key);

      auto page = slow_get_page(key);
      frequenters_.insert({key, page});

      return false;
    }

    if (max_capacity() == recents_.size() + evicted_recents_.size()) {
      if (recents_.size() < max_capacity()) {
        evicted_recents_.pop_last_recently_used();
        replace(key);
      } else {
        recents_.pop_last_recently_used();
      }
    } else {
      auto current_size = recents_.size() + frequenters_.size() + evicted_recents_.size() +
                          evicted_frequenters_.size();
      if (current_size >= max_capacity()) {
        if (current_size == 2 * max_capacity()) {
          evicted_frequenters_.pop_last_recently_used();
        }
        replace(key);
      }
    }

    auto page = slow_get_page(key);
    recents_.insert({key, page});

    return false;
  }
};

}  // namespace caches

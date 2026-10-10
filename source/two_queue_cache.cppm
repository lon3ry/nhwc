module;

#include <cstddef>
#include <functional>

export module two_queue_cache;

import lru_queue;
import base_cache;

namespace caches {

export template <typename KeyT, typename T>
class TwoQueueCache final : public BaseCache<KeyT, T> {
public:
  explicit TwoQueueCache(std::size_t capacity) :
    BaseCache<KeyT, T>(capacity), kin_(capacity / 4), kout_(capacity / 2) {};

private:
  using BaseCache<KeyT, T>::max_capacity;

  std::size_t kin_, kout_;

  LRUQueue<KeyT, T> am_, a1_in_;
  GhostLRUQueue<KeyT> a1_out_;

  bool page_slots_available() const { return a1_in_.size() + am_.size() < max_capacity(); }
  bool a1_in_above_threshold() const { return a1_in_.size() > kin_; }
  bool a1_out_above_threshold() const { return a1_out_.size() > kout_; }

  void reclaim_for() {
    if (page_slots_available()) {
      return;
    } else if (a1_in_above_threshold()) {
      auto item = a1_in_.pop_last_recently_used();
      a1_out_.insert(item->key);
      if (a1_out_above_threshold())
        a1_out_.pop_last_recently_used();
    } else {
      am_.pop_last_recently_used();
    }
  }

  bool do_lookup_update(const KeyT& key, std::function<T(KeyT)> slow_get_page) override {
    auto hit_am = am_.lookup(key);
    if (hit_am)
      return true;

    auto hit_a1_out = a1_out_.has(key);
    if (hit_a1_out) {
      a1_out_.erase(key);
      reclaim_for();
      am_.insert({key, slow_get_page(key)});
      return false;
    }

    auto hit_a1_in = a1_in_.has(key);
    if (hit_a1_in)
      return true;

    reclaim_for();

    a1_in_.insert({key, slow_get_page(key)});

    return false;
  }
};

}  // namespace caches

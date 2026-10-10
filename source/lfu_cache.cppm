module;

#include <cstddef>
#include <functional>
#include <list>
#include <unordered_map>

export module lfu_cache;

import base_cache;

namespace caches {

export template <typename Key, typename Value>
class LFUCache final : public BaseCache<Key, Value> {
public:
  using BaseCache<Key, Value>::max_capacity;
  LFUCache(std::size_t capacity) : BaseCache<Key, Value>(capacity), min_freq_(1) {}

  bool is_full() const { return max_capacity() == cache_map_.size(); }

private:
  std::size_t min_freq_;

  struct Record {
    Key key;
    Value page;
    std::size_t freq;
  };

  using NodeIt = typename std::list<Record>::iterator;
  std::unordered_map<Key, NodeIt> cache_map_;
  std::unordered_map<size_t, std::list<Record>> freq_to_list_map_;

  void check_freq_bucket_for_emptiness(std::size_t freq) {
    if (freq_to_list_map_[freq].empty()) {
      freq_to_list_map_.erase(freq);
    }
  }

  bool do_lookup_update(const Key& key, std::function<Value(Key)> slow_get_page) {
    if (auto it = cache_map_.find(key); it != cache_map_.end()) {
      const std::size_t old_freq = it->second->freq;
      auto& new_bucket = freq_to_list_map_[old_freq + 1];
      new_bucket.splice(new_bucket.begin(), freq_to_list_map_[old_freq], it->second);
      it->second->freq += 1;

      check_freq_bucket_for_emptiness(old_freq);
      if (!freq_to_list_map_.contains(min_freq_))
        min_freq_++; // element can be promoted only 1 bucket upper

      return true;
    } else {
      if (is_full()) {
        auto victim = freq_to_list_map_[min_freq_].back();
        cache_map_.erase(victim.key);
        freq_to_list_map_[min_freq_].pop_back();
        check_freq_bucket_for_emptiness(min_freq_); // no need to increase min_freq as we set it 1
      }                                             // later anyway

      auto page = slow_get_page(key);

      min_freq_ = 1;
      auto& min_freq_bucket = freq_to_list_map_[min_freq_];
      min_freq_bucket.emplace_front(key, std::move(page), 1);
      cache_map_.emplace(key, min_freq_bucket.begin());

      return false;
    }
  }
};

}  // namespace caches

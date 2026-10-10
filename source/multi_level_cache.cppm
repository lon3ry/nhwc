module;

#include <cstddef>
#include <functional>
#include <vector>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

export module multi_level_cache;

import arc_cache;
import lirs_cache;
import two_queue_cache;
import lfu_cache;
import lru_cache;
import base_cache;

namespace caches {

export enum class CacheType { kARC, k2Q, kLRU, kLFU, kLIRS };

export struct CacheLevel {
  CacheType type;
  std::size_t capacity;
};

export CacheType string_to_cache_type(const std::string_view str) {
  if (str == "LRU")  return CacheType::kLRU;
  if (str == "ARC")  return CacheType::kARC;
  if (str == "2Q")   return CacheType::k2Q;
  if (str == "LFU")  return CacheType::kLFU;
  if (str == "LIRS") return CacheType::kLIRS;
  throw std::invalid_argument("unknown cache type: " + std::string(str));
}

export template <typename Key, typename Value>
class MultiLevelCache {
public:
  explicit MultiLevelCache(std::ranges::sized_range auto&& levels) {
    cache_.reserve(std::ranges::size(levels));
    for (const auto& level : levels) {
      switch (level.type) {
        case CacheType::kARC:
          cache_.emplace_back(std::make_unique<ARCCache<Key, Value>>(level.capacity));
          break;
        case CacheType::k2Q:
          cache_.emplace_back(std::make_unique<TwoQueueCache<Key, Value>>(level.capacity));
          break;
        case CacheType::kLRU:
          cache_.emplace_back(std::make_unique<LRUCache<Key, Value>>(level.capacity));
          break;
        case CacheType::kLFU:
          cache_.emplace_back(std::make_unique<LFUCache<Key, Value>>(level.capacity));
          break;
        case CacheType::kLIRS:
          cache_.emplace_back(std::make_unique<LIRSCache<Key, Value>>(level.capacity));
          break;
      }
    }
  }

  bool lookup_update(const Key& key, std::function<Value(Key)> slow_get_page) {
    bool loaded = false;
    Value page;

    auto get_page = [&](Key key) -> Value {
      if (!loaded) {
        page = slow_get_page(key);
        loaded = true;
      }

      return page;
    };

    for (const auto& level : cache_) {
      auto hit = level->lookup_update(key, get_page);
      if (hit) return true;
    }
    return false;
  }

private:
  std::vector<std::unique_ptr<BaseCache<Key, Value>>> cache_;
};

}  // namespace caches

#pragma once

#include <functional>
#include <list>
#include <cstddef>
#include <memory>
#include <limits>

#include "base_cache.hpp"
#include "arc_cache.hpp"
#include "lru_cache.hpp"
#include "lfu_cache.hpp"
#include "lirs_cache.hpp"
#include "two_queue_cache.hpp"

namespace caches
{

enum class CacheType
{
    ARC,
    TWO_QUEUE,
    LRU,
    LFU,
    LIRS
};

struct CacheLevel
{
    CacheType type;
    std::size_t capacity;
};

CacheType string_to_cache_type(const std::string_view str);

template <typename T, typename KeyT = int> class MultiLevelCache
{
    std::list<std::unique_ptr<BaseCache<T, KeyT>>> cache_;

public:
    MultiLevelCache(std::list<CacheLevel> levels)
    {
        for (auto level : levels)
        {
            switch (level.type)
            {
                case CacheType::ARC:
                    cache_.emplace_back(std::make_unique<ARCCache<T, KeyT>>(level.capacity));
                    break;
                case CacheType::TWO_QUEUE:
                    cache_.emplace_back(std::make_unique<TwoQueueCache<T, KeyT>>(level.capacity));
                    break;
                case CacheType::LRU:
                    cache_.emplace_back(std::make_unique<LRUCache<T, KeyT>>(level.capacity));
                    break;
                case CacheType::LFU:
                    cache_.emplace_back(std::make_unique<LFUCache<T, KeyT>>(level.capacity));
                    break;
                case CacheType::LIRS:
                    cache_.emplace_back(std::make_unique<LIRSCache<T, KeyT>>(level.capacity));
                    break;
            }
        }
    }

    bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page)
    {
        for (const auto& level : cache_)
        {
            // TODO: Here is a high perfomance issue, because we call slow_get_page() for each level
            // that misses it before we meet a level which doesn't. It's pointeless, since we can
            // get it once and then copy it forward to each next level. Still, it doesn't affect
            // cache hit ratio, so we can test everything even with this flaw implementation
            auto hit = level->lookup_update(key, slow_get_page);
            if (hit)
                return true;
        }
        return false;
    }
};

} // namespace caches

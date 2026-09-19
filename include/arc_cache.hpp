#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>

#include "lru_queue.hpp"
#include "ghost_lru_queue.hpp"

namespace caches
{

template <typename T, typename KeyT = int> class ARCCache
{
public:
    explicit ARCCache(std::size_t capacity) : capacity_(capacity) {}
    bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page);

private:
    const std::size_t capacity_;

    LRUQueue<T, KeyT> recents_;               // T1: pages seen only once recently
    LRUQueue<T, KeyT> frequenters_;           // T2: pages seen at least twice recently
    GhostLRUQueue<KeyT> evicted_recents_;     // B1: ghost cache for T1
    GhostLRUQueue<KeyT> evicted_frequenters_; // B2: ghost cache for T2

    std::size_t p_ = 0;

    void replace(KeyT key);
};

template <typename T, typename KeyT>
bool ARCCache<T, KeyT>::lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page)
{
    bool hit_recents = recents_.lookup(key);
    if (hit_recents)
    {
        auto item = recents_.pop_most_recently_used();
        frequenters_.insert(item->first, item->second);
        return true;
    }

    bool hit_frequenters = frequenters_.lookup(key);
    if (hit_frequenters)
        return true;

    bool hit_evicted_recents = evicted_recents_.lookup(key);
    if (hit_evicted_recents)
    {
        std::size_t delta1 = evicted_recents_.size() >= evicted_frequenters_.size() ? 1 :
                                evicted_frequenters_.size() / evicted_recents_.size();
        p_ = std::min(p_ + delta1, capacity_);

        replace(key);
        evicted_recents_.erase(key);

        auto page = slow_get_page(key);
        frequenters_.insert(key, page);

        return false;
    }

    bool hit_evicted_frequenters = evicted_frequenters_.lookup(key);
    if (hit_evicted_frequenters)
    {
        std::size_t delta2 = evicted_frequenters_.size() >= evicted_recents_.size() ? 1 :
                                evicted_recents_.size() / evicted_frequenters_.size();
        // p_ = std::max<long long>(p_ - delta2, 0);
        p_ = (p_ > delta2) ? (p_ - delta2) : 0;

        replace(key);
        evicted_frequenters_.erase(key);

        auto page = slow_get_page(key);
        frequenters_.insert(key, page);

        return false;
    }

    if (capacity_ == recents_.size() + evicted_recents_.size())
    {
        if (recents_.size() < capacity_)
        {
            evicted_recents_.pop_last_recently_used();
            replace(key);
        }
        else
        {
            recents_.pop_last_recently_used();
        }
    }
    else
    {
        auto current_size = recents_.size() + frequenters_.size() + evicted_recents_.size() +
                            evicted_frequenters_.size();
        if (current_size >= capacity_)
        {
            if (current_size == 2 * capacity_)
                evicted_frequenters_.pop_last_recently_used();

            replace(key);
        }
    }

    auto page = slow_get_page(key);
    recents_.insert(key, page);

    return false;
}

template <typename T, typename KeyT>
void ARCCache<T, KeyT>::replace(KeyT key)
{
    if (!recents_.empty() && (recents_.size() > p_ || (evicted_frequenters_.has(key) &&
        p_ == recents_.size())))
    {
        auto item = recents_.pop_last_recently_used();
        evicted_recents_.insert(item->first);
    }
    else
    {
        auto item = frequenters_.pop_last_recently_used();
        evicted_frequenters_.insert(item->first);
    }
}

} // namespace caches

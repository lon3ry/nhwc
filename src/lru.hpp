#pragma once

#include <cstddef>
#include <list>
#include <unordered_map>

namespace caches
{

template <typename T, typename KeyT = int> class LRUCache
{
    // Each entry is {key, page}; most recently used entry is at the front.
    std::list<std::pair<KeyT, T>> cache_;

    using ListIt = typename std::list<std::pair<KeyT, T>>::iterator;
    std::unordered_map<KeyT, ListIt> hash_;
    bool full() const { return (cache_.size() == size_); }

public:
    explicit LRUCache(std::size_t size) : size_(size) {}
    std::size_t max_capacity() const { return size_; }
    template <typename F> bool lookup_update(KeyT key, F slow_get_page)
    {
        if (max_capacity() == 0)
            return false;

        auto hit = hash_.find(key);
        if (hit != hash_.end())
        {
            auto eltit = hit->second;
            cache_.splice(cache_.begin(), cache_, eltit);
            return true;
        }

        T page = slow_get_page(key);

        if (full())
        {
            hash_.erase(cache_.back().first);
            cache_.pop_back();
        }
        cache_.emplace_front(key, page);
        hash_.emplace(key, cache_.begin());
        return false;
    }
};

} // namespace caches
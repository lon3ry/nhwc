#pragma once

#include <list>
#include <unordered_map>
#include <cstddef>

namespace caches
{

template <typename T, typename KeyT = int> class LRUList
{
    // Each entry is {key, page}; most recently used entry is at the front.
    std::list<std::pair<KeyT, T>> cache_;

    using ListIt = typename std::list<std::pair<KeyT, T>>::iterator;
    std::unordered_map<KeyT, ListIt> hash_;
public:
    std::size_t size() { return cache_.size(); }
    std::size_t empty() { return size() == 0; }

    bool lookup(KeyT key)
    {
        auto hit = hash_.find(key);
        if (hit != hash_.end())
        {
            auto eltit = hit->second;
            cache_.splice(cache_.begin(), cache_, eltit);
            return true;
        }
        return false;
    }

    void insert(KeyT key, T page)
    {
        cache_.emplace_front(key, page);
        hash_.emplace(key, cache_.begin());
    }

    T pop_last_recently_used()
    {
        hash_.erase(cache_.back().first);
        auto item = cache_.back().second;
        cache_.pop_back();
        return item;
    }

    T pop_most_recently_used()
    {
        hash_.erase(cache_.front().first);
        auto item = cache_.front().second;
        cache_.pop_front();
        return item;
    }
};

} // namespace caches

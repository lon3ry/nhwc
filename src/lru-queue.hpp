#pragma once

#include <list>
#include <unordered_map>
#include <optional>
#include <cstddef>

namespace caches
{

template <typename T, typename KeyT = int> class LRUQueue
{
    // Each entry is {key, page}; most recently used entry is at the front.
    std::list<std::pair<KeyT, T>> cache_;

    using QueueItem = typename std::pair<KeyT, T>;
    using QueueIt = typename std::list<QueueItem>::iterator;
    std::unordered_map<KeyT, QueueIt> hash_;

public:
    std::size_t size() const { return cache_.size(); }
    bool empty() const { return size() == 0; }

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

    std::optional<QueueItem> pop_most_recently_used()
    {
        if (empty())
            return std::nullopt;

        hash_.erase(cache_.front().first);
        auto item = cache_.front();
        cache_.pop_front();

        return item;
    }

    std::optional<QueueItem> pop_last_recently_used()
    {
        if (empty())
            return std::nullopt;

        hash_.erase(cache_.back().first);
        auto item = cache_.back();
        cache_.pop_back();

        return item;
    }
};

} // namespace caches

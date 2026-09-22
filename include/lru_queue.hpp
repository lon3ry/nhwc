#pragma once

#include <list>
#include <unordered_map>
#include <optional>
#include <cstddef>

namespace caches
{

template <typename T, typename KeyT = int> class LRUQueue
{
    struct QueueItem
    {
        KeyT key;
        T page;
    };

    std::list<QueueItem> cache_;

    using QueueIt = typename std::list<QueueItem>::iterator;
    std::unordered_map<KeyT, QueueIt> hash_;

public:
    std::size_t size() const { return cache_.size(); }
    bool empty() const { return size() == 0; }
    bool has(KeyT key) const { return hash_.find(key) != hash_.end(); }

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

        hash_.erase(cache_.front().key);
        auto item = cache_.front();
        cache_.pop_front();

        return item;
    }

    std::optional<QueueItem> pop_last_recently_used()
    {
        if (empty())
            return std::nullopt;

        hash_.erase(cache_.back().key);
        auto item = cache_.back();
        cache_.pop_back();

        return item;
    }
};

} // namespace caches

#pragma once

#include <list>
#include <unordered_map>
#include <optional>
#include <cstddef>

namespace caches
{

// GhostCache does not store the page data itselfs, only the key
template <typename KeyT = int> class GhostLRUQueue
{
public:
    std::size_t size() const { return cache_.size(); }
    bool empty() const { return size() == 0; }
    bool has(KeyT key) const { return hash_.find(key) != hash_.end(); }

    bool lookup(KeyT key);
    void erase(KeyT key);
    void insert(KeyT key);
    std::optional<KeyT> pop_most_recently_used();
    std::optional<KeyT> pop_last_recently_used();

private:
    using CacheList = std::list<KeyT>;
    CacheList cache_;

    using CacheListIt = typename CacheList::iterator;
    std::unordered_map<KeyT, CacheListIt> hash_;
};

template <typename KeyT>
bool GhostLRUQueue<KeyT>::lookup(KeyT key)
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

template <typename KeyT>
void GhostLRUQueue<KeyT>::erase(KeyT key)
{
    auto hit = hash_.find(key);
    if (hit == hash_.end())
        return;
    auto eltit = hit->second;
    cache_.erase(eltit);
    hash_.erase(hit);
}

template <typename KeyT>
void GhostLRUQueue<KeyT>::insert(KeyT key)
{
    cache_.emplace_front(key);
    hash_.emplace(key, cache_.begin());
}

template <typename KeyT>
std::optional<KeyT> GhostLRUQueue<KeyT>::pop_most_recently_used()
{
    if (empty())
        return std::nullopt;

    hash_.erase(cache_.front());
    auto key = cache_.front();
    cache_.pop_front();

    return key;
}

template <typename KeyT>
std::optional<KeyT> GhostLRUQueue<KeyT>::pop_last_recently_used()
{
    if (empty())
        return std::nullopt;

    hash_.erase(cache_.back());
    auto key = cache_.back();
    cache_.pop_back();

    return key;
}


} // namespace caches

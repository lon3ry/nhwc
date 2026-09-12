#include <list>
#include <unordered_map>
#include <cstddef>

namespace caches
{

// GhostCache does not store the page data itselfs, only the key
template <typename KeyT = int> class GhostLRUCache
{
    using CacheList = std::list<KeyT>;
    CacheList cache_;

    using CacheListIt = typename CacheList::iterator;
    std::unordered_map<KeyT, CacheListIt> hash_;
public:
    std::size_t size() { return cache_.size(); }
    bool has(KeyT key) { return hash_.find(key) != hash_.end(); }

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

    void insert(KeyT key)
    {
        cache_.emplace_front(key);
        hash_.emplace(key, cache_.begin());
    }

    KeyT pop_most_recently_used()
    {
        hash_.erase(cache_.front());
        auto key = cache_.front();
        cache_.pop_front();
        return key;
    }

    KeyT pop_last_recently_used()
    {
        hash_.erase(cache_.back());
        auto key = cache_.back();
        cache_.pop_back();
        return key;
    }
};

} // namespace caches

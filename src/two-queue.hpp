#pragma once

#include <cstddef>
#include <list>
#include <unordered_map>

namespace caches
{

template <typename T, typename KeyT = int> class SimplifiedTwoQueueCache
{
    std::size_t size_;
    std::size_t al_size_;
    std::size_t am_size_;
    std::list<std::pair<KeyT, T>> al_;
    std::list<std::pair<KeyT, T>> am_;

    using CacheList = typename std::list<std::pair<KeyT, T>>;
    using CacheListIt = typename CacheList::iterator;
    using LRUIt = CacheListIt;
    using QueueIt = CacheListIt;
    std::unordered_map<KeyT, LRUIt> al_hash_;
    std::unordered_map<KeyT, QueueIt> am_hash_;
    bool full_am() const { return am_.size() == am_size_; }
    bool full_al() const { return al_.size() == al_size_; }
public:
    explicit SimplifiedTwoQueueCache(std::size_t size) : size_(size), 
        al_size_(size / 4),
        am_size_(size - (size / 4)) 
    {}
    
    template <typename F> bool lookup_update(KeyT key, F slow_get_page)
    {
        auto hit_am = am_hash_.find(key);
        if (hit_am != am_hash_.end())
        {
            auto eltit = hit_am->second;
            am_.splice(am_.begin(), am_, eltit);
            return true;
        }
        
        auto hit_al = al_hash_.find(key);
        if (hit_al != al_hash_.end())
        {
            auto eltit = hit_al->second;
            auto elem = *eltit;
            al_.erase(eltit);
            al_hash_.erase(hit_al);
            if (full_am())
            {
                am_hash_.erase(am_.back().first);
                am_.pop_back();
            }
            am_.emplace_front(elem.first, elem.second);
            am_hash_.emplace(key, am_.begin());
            return true;
        }
        
        T page = slow_get_page(key);
        if (!full_al())
        {
            al_.emplace_front(key, page);
            al_hash_.emplace(key, al_.begin());
        }
        else if (!full_am())
        {
            al_hash_.erase(al_.back().first);
            al_.pop_back();
            al_.emplace_front(key, page);
            al_hash_.emplace(key, al_.begin());
        }
        else
        {
            am_hash_.erase(am_.back().first);
            am_.pop_back();
            al_.emplace_front(key, page);
            al_hash_.emplace(key, al_.begin());
        }
        return false;
    }
};

} //namespace caches

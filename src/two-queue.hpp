#pragma once

#include <cstddef>
#include <list>
#include <unordered_map>

namespace caches
{

/*
template <typename T, typename KeyT = int> class SimplifiedTwoQueueCache
{    
    using CacheList = typename std::list<std::pair<KeyT, T>>;
    using CacheListIt = typename CacheList::iterator;
    using HashMap = typename std::unordered_map<KeyT, CacheListIt>;
    CacheList al_, am_;
    std::size_t capacity_, al_threshold_;
    HashMap al_hash_, am_hash_;
    bool page_slots_available() const { return al_.size() + am_.size() < capacity_; }
    bool al_above_threshold() const { return al_.size() > al_threshold_; }
public:
    explicit SimplifiedTwoQueueCache(std::size_t size) :
        capacity_(size),
        al_threshold_(size / 4) 
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
            if (!page_slots_available())
            {
                am_hash_.erase(am_.back().first);
                am_.pop_back();
            }
            am_.emplace_front(elem.first, elem.second);
            am_hash_.emplace(key, am_.begin());
            return true;
        }
        
        T page = slow_get_page(key);
        if (page_slots_available())
        {
            al_.emplace_front(key, page);
            al_hash_.emplace(key, al_.begin());
        }
        else if (al_above_threshold())
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
*/

template <typename T, typename KeyT = int> class FullTwoQueueCache
{
    using CacheList = typename std::list<std::pair<KeyT, T>>;
    using KeyList = typename std::list<KeyT>;
    using CacheListIt = typename CacheList::iterator;
    using KeyListIt = typename KeyList::iterator;
    using CacheHashMap = typename std::unordered_map<KeyT, CacheListIt>;
    using KeyHashMap = typename std::unordered_map<KeyT, KeyListIt>;
    CacheList al_in_, am_;
    KeyList al_out_;
    std::size_t capacity_, kin_, kout_;
    CacheHashMap al_in_hash_, am_hash_;
    KeyHashMap al_out_hash_;
    bool page_slots_available() const { return al_in_.size() + am_.size() < capacity_; }
    bool al_in_above_threshold() const { return al_in_.size() > kin_; }
    bool al_out_above_threshold() const { return al_out_.size() > kout_; }
    
    void reclaim_for()
    {
        if (page_slots_available()) 
        {
            return;
        }
        else if (al_in_above_threshold())
        {
            auto y = al_in_.back().first;
            al_in_hash_.erase(y);
            al_in_.pop_back();
            al_out_.emplace_front(y);
            al_out_hash_.emplace(y, al_out_.begin());
            if (al_out_above_threshold())
            {
                al_out_hash_.erase(al_out_.back());
                al_out_.pop_back();
            }
        }
        else
        {
            am_hash_.erase(am_.back().first);
            am_.pop_back();
        }
    }
public:
    explicit FullTwoQueueCache(std::size_t size) : 
        capacity_(size),
        kin_(size / 4),
        kout_(size / 2)
    {};
    template <typename F> bool lookup_update(KeyT key, F slow_get_page)
    {
        auto hit_am = am_hash_.find(key);
        if (hit_am != am_hash_.end())
        {
            auto eltit = hit_am->second;
            am_.splice(am_.begin(), am_, eltit);
            return true;
        }

        auto hit_al_out = al_out_hash_.find(key);
        if (hit_al_out != al_out_hash_.end())
        {        
            al_out_.erase(hit_al_out->second);
            al_out_hash_.erase(hit_al_out);
            reclaim_for();
            T page = slow_get_page(key);
            am_.emplace_front(key, page);
            am_hash_.emplace(key, am_.begin());
            return true;
        }

        auto hit_al_in = al_in_hash_.find(key);
        if (hit_al_in != al_in_hash_.end())
        {
            return true;
        }

        reclaim_for();
        T page = slow_get_page(key);
        al_in_.emplace_front(key, page);
        al_in_hash_.emplace(key, al_in_.begin());
        return false;
    }
};

} //namespace caches

#pragma once

#include <cstddef>
#include <functional>

#include "base_cache.hpp"
#include "lru_queue.hpp"
#include "ghost_lru_queue.hpp"

namespace caches
{

template <typename T, typename KeyT = int> class TwoQueueCache : public BaseCache<T, KeyT>
{
    std::size_t capacity_, kin_, kout_;

    LRUQueue<T, KeyT> am_, a1_in_;
    GhostLRUQueue<KeyT> a1_out_;

    bool page_slots_available() const { return a1_in_.size() + am_.size() < capacity_; }
    bool a1_in_above_threshold() const { return a1_in_.size() > kin_; }
    bool a1_out_above_threshold() const { return a1_out_.size() > kout_; }

    void reclaim_for()
    {
        if (page_slots_available())
        {
            return;
        }
        else if (a1_in_above_threshold())
        {
            auto item = a1_in_.pop_last_recently_used();
            a1_out_.insert(item->first);
            if (a1_out_above_threshold())
                a1_out_.pop_last_recently_used();
        }
        else
        {
            am_.pop_last_recently_used();
        }
    }

public:
    explicit TwoQueueCache(std::size_t capacity) :
        capacity_(capacity),
        kin_((capacity + 3) / 4), // avoid kin_ = 0
        kout_((capacity + 1) / 2) // same for kout_
    {};

    bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page)
    {
        auto hit_am = am_.lookup(key);
        if (hit_am)
            return true;

        auto hit_a1_out = a1_out_.has(key);
        if (hit_a1_out)
        {
            a1_out_.erase(key);
            reclaim_for();
            T page = slow_get_page(key);
            am_.insert(key, page);
            return false;
        }

        auto hit_a1_in = a1_in_.has(key);
        if (hit_a1_in)
            return true;

        reclaim_for();

        T page = slow_get_page(key);
        a1_in_.insert(key, page);

        return false;
    }

};

} //namespace caches

#pragma once

#include <cstddef>
#include <list>
#include <unordered_map>

#include "lru-queue.hpp"
#include "ghost-lru-queue.hpp"

namespace caches
{

template <typename T, typename KeyT = int> class TwoQueueCache
{
    std::size_t capacity_, kin_, kout_;

    LRUQueue<T, KeyT> am_, al_in_;
    GhostLRUQueue<KeyT> al_out_;

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
            auto item = al_in_.pop_last_recently_used();
            al_out_.insert(item->first);
            if (al_out_above_threshold())
                al_out_.pop_last_recently_used();
        }
        else
        {
            am_.pop_last_recently_used();
        }
    }

public:
    explicit TwoQueueCache(std::size_t capacity) :
        capacity_(capacity),
        kin_((capacity + 3) / 4),
        kout_((capacity + 1) / 2)
    {};

    template <typename F> bool lookup_update(KeyT key, F slow_get_page)
    {
        auto hit_am = am_.lookup(key);
        if (hit_am)
            return true;

        auto hit_al_out = al_out_.has(key);
        if (hit_al_out)
        {
            al_out_.erase(key);
            reclaim_for();
            T page = slow_get_page(key);
            am_.insert(key, page);
            return false;
        }

        auto hit_al_in = al_in_.has(key);
        if (hit_al_in)
            return true;

        reclaim_for();

        T page = slow_get_page(key);
        al_in_.insert(key, page);

        return false;
    }
};

} //namespace caches

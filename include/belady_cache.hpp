#pragma once

#include <cstddef>
#include <unordered_map>
#include <list>
#include <vector>
#include <algorithm>

namespace caches
{

template <typename T, typename KeyT>
class BeladyCache
{

    const std::size_t capacity_;

    struct Record
    {
        KeyT key;
        T page;
    };

    using NodeIt = typename std::list<Record>::iterator;
    using VecIt = typename std::vector<T>::const_iterator;

    NodeIt find_free_space(VecIt seq_begin, VecIt seq_end)
    {
        NodeIt victim_it = cache_list_.begin();
        VecIt victim_request_it = seq_begin;

        for (auto cur_page_it = cache_list_.begin(); cur_page_it != cache_list_.end();
                                                                                    ++cur_page_it)
        {
            auto next_it = find(seq_begin, seq_end, cur_page_it->key);
            if (next_it == seq_end)
                return cur_page_it;
            else
            {
                if (next_it > victim_request_it)
                {
                    victim_request_it = next_it;
                    victim_it = cur_page_it;
                }
            }
        }

        return victim_it;
    }

    std::unordered_map<KeyT, NodeIt> cache_map_;
    std::list<Record> cache_list_;

public:
    BeladyCache(std::size_t capacity) : capacity_(capacity) {}

    std::size_t max_capacity() const { return capacity_; }
    bool is_full() const { return (capacity_ == cache_map_.size()); }

    // do we need size_t here?
    std::size_t calculate_hits(const std::vector<T> page_request_sequence)
    {
        if (max_capacity() == 0)
            return 0; // no hits possible

        std::size_t hits = 0;

        for (auto it = page_request_sequence.begin(); it != page_request_sequence.end(); ++it)
        {
            KeyT key = *it;
            if (auto page_it = cache_map_.find(key); page_it != cache_map_.end())
            {
                hits++;
            }
            else
            {
                if (is_full())
                {
                    auto node_it = find_free_space(it, page_request_sequence.end());
                    cache_map_.erase(node_it->key);
                    cache_list_.erase(node_it);
                }

                Record new_page = {.key = key, .page = key}; // add slow get page
                cache_list_.push_front(new_page);
                cache_map_[key] = cache_list_.begin();
            }
        }
        return hits;
    }
};


} // namespace caches

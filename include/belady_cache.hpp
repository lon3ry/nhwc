#pragma once

#include <cstddef>
#include <unordered_map>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>


template <typename T, typename KeyT>
class BeladyCache
{
public:
    BeladyCache(std::size_t capacity) : capacity_(capacity) {}

    std::size_t max_capacity() const { return capacity_; }
    bool is_full() const { return (capacity_ == cache_map_.size()); }

    std::size_t calculate_hits(const std::vector<T> page_request_sequence); // do we need size_t here?

private:
    const std::size_t capacity_;

    struct Record
    {
        KeyT key;
        T page;
    };

    using NodeIt = typename std::list<Record>::iterator;
    using VecIt = typename std::vector<T>::const_iterator;

    NodeIt find_free_space(VecIt seq_begin, VecIt seq_end);
    std::unordered_map<KeyT, NodeIt> cache_map_;
    std::list<Record> cache_list_;
};


template <typename T, typename KeyT>
std::size_t BeladyCache<T, KeyT>::calculate_hits(const std::vector<T> page_request_sequence)
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

template <typename T, typename KeyT>
typename BeladyCache<T, KeyT>::NodeIt BeladyCache<T, KeyT>::find_free_space(
        typename BeladyCache<T, KeyT>::VecIt seq_begin,
        typename BeladyCache<T, KeyT>::VecIt seq_end)
{
    NodeIt candidate_it = cache_list_.begin();
    VecIt candidate_request_it = seq_begin;

    for (auto cur_page_it = cache_list_.begin(); cur_page_it != cache_list_.end(); ++cur_page_it)
    {
        auto next_iter = find(seq_begin, seq_end, cur_page_it->key);
        if (next_iter == seq_end)
            return cur_page_it;
        else
        {
            if (next_iter > candidate_request_it)
            {
                candidate_request_it = next_iter;
                candidate_it = cur_page_it;
            }
        }
    }

    return candidate_it;
}

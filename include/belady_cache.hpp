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

    std::unordered_map<KeyT, NodeIt> cache_map_;
    std::list<Record> cache_list_;

    std::unordered_map<KeyT, std::list<std::size_t>> page_request_ind_map;

    void init_page_request_ind_map(const std::vector<T> &page_request_sequence)
    {
        std::size_t request_vec_sz = page_request_sequence.size();
        for (std::size_t i = 0; i < request_vec_sz; ++i)
        {
            KeyT cur_page_key = page_request_sequence[i];
            page_request_ind_map[cur_page_key].push_back(i);
        }
    }

    NodeIt find_free_space()
    {
        NodeIt victim_it = cache_list_.begin();
        std::size_t farthest_ind = 0;

        for (auto it = cache_list_.begin(); it != cache_list_.end(); ++it)
        {
            auto &cur_ind_list = page_request_ind_map[it->key];
            if (cur_ind_list.empty())
                return it;

            std::size_t next_ind = cur_ind_list.front();
            if (next_ind > farthest_ind)
            {
                farthest_ind = next_ind;
                victim_it = it;
            }
        }

        return victim_it;
    }

public:
    BeladyCache(std::size_t capacity) : capacity_(capacity) {}

    std::size_t max_capacity() const { return capacity_; }
    bool is_full() const { return (capacity_ == cache_map_.size()); }

    // do we need size_t here?
    std::size_t calculate_hits(const std::vector<T> &page_request_sequence)
    {
        if (max_capacity() == 0)
            return 0; // no hits possible

        init_page_request_ind_map(page_request_sequence);

        std::size_t hits = 0;

        for (std::size_t i = 0; i < page_request_sequence.size(); ++i)
        {
            KeyT key = page_request_sequence[i];

            page_request_ind_map[key].pop_front();

            if (auto page_it = cache_map_.find(key); page_it != cache_map_.end())
            {
                hits++;
            }
            else
            {
                if (is_full())
                {
                    auto node_it = find_free_space();
                    cache_map_.erase(node_it->key);
                    cache_list_.erase(node_it);
                }

                Record new_page = {.key = key, .page = key}; // add slow get page?
                cache_list_.push_front(new_page);
                cache_map_[key] = cache_list_.begin();
            }
        }
        return hits;
    }
};


} // namespace caches

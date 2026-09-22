#pragma once

#include <cstddef>
#include <unordered_map>
#include <list>
#include <functional>

#include "base_cache.hpp"

namespace caches
{

template <typename T, typename KeyT> class LFUCache : public BaseCache<T, KeyT>
{
public:
    LFUCache(std::size_t capacity) : capacity_(capacity), min_freq_(1) {}

    std::size_t max_capacity() const { return capacity_; }
    bool is_full() const { return (capacity_ == cache_map_.size()); }

    bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page);

private:
    const std::size_t capacity_;
    int min_freq_;

    struct Record
    {
        KeyT key;
        T page;
        unsigned int freq;
    };

    using NodeIt = typename std::list<Record>::iterator;
    std::unordered_map<KeyT, NodeIt> cache_map_;
    std::unordered_map<unsigned int, std::list<Record>> freq_to_list_map_;
};


template <typename T, typename KeyT>
bool LFUCache<T, KeyT>::lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page)
{
    if (max_capacity() == 0)
        return false;

    if (auto it = cache_map_.find(key); it != cache_map_.end())
    {
        Record node = *(it->second);
        freq_to_list_map_[node.freq].erase(it->second);
        node.freq += 1;

        freq_to_list_map_[node.freq].push_front(node);
        cache_map_[key] = freq_to_list_map_[node.freq].begin();

        if (freq_to_list_map_[min_freq_].empty())
            min_freq_++;

        return true;
    }
    else
    {
        if (is_full())
        {
            auto evicted_node = freq_to_list_map_[min_freq_].back();
            cache_map_.erase(evicted_node.key);
            freq_to_list_map_[min_freq_].pop_back();
        }

        auto page = slow_get_page(key);
        Record rec { .key = key, .page = page, .freq = 1 };

        min_freq_ = 1;
        freq_to_list_map_[min_freq_].push_front(rec);
        cache_map_[key] = freq_to_list_map_[min_freq_].begin();

        return false;
    }
}

} // namespace caches

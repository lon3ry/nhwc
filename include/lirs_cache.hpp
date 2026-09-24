#pragma once

#include <functional>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <list>
#include <unordered_map>

#include "base_cache.hpp"

namespace caches
{

template <typename T, typename KeyT = int> class LIRSCache : public BaseCache<T, KeyT>
{
    std::size_t capacity_, lirs_max_;

    std::list<T> cache_;

    enum class BlockStatus { LIR, HIR };
    using CacheIt = typename std::list<T>::iterator;

    struct StackRecord
    {
        KeyT key;
        CacheIt data;
        BlockStatus status;

        StackRecord(KeyT k, CacheIt d, BlockStatus s) : key(k), data(d), status(s) {}
    };

    struct QueueRecord
    {
        KeyT key;
        CacheIt data;

        QueueRecord(KeyT k, CacheIt d) : key(k), data(d) {}
    };

    using StackIt = typename std::list<StackRecord>::iterator;
    using QueueIt = typename std::list<QueueRecord>::iterator;

    std::list<StackRecord> stack_;
    std::list<QueueRecord> queue_;
    std::unordered_map<KeyT, StackIt> hash_stack_;
    std::unordered_map<KeyT, QueueIt> hash_queue_;

    std::size_t lir_count_ = 0;

    void prune_stack()
    {
        while (!stack_.empty() && stack_.back().status == BlockStatus::HIR)
        {
            hash_stack_.erase(stack_.back().key);
            stack_.pop_back();
        }
    }

    void demote_lir_bottom()
    {
        if (stack_.empty() || stack_.back().status != BlockStatus::LIR)
            return;

        StackRecord bottom = stack_.back();
        stack_.pop_back();
        hash_stack_.erase(bottom.key);
        bottom.status = BlockStatus::HIR;
        queue_.emplace_front(bottom.key, bottom.data);
        hash_queue_.emplace(bottom.key, queue_.begin());
        --lir_count_;
    }

    bool is_full() const { return lir_count_ + queue_.size() >= capacity_; }

    void evict_lru_hir()
    {
        if (queue_.empty())
            return;

        QueueRecord victim = queue_.back();
        queue_.pop_back();
        hash_queue_.erase(victim.key);

        auto it = hash_stack_.find(victim.key);
        if (it != hash_stack_.end())
            it->second->data = cache_.end();

        cache_.erase(victim.data);
    }

public:
    explicit LIRSCache(std::size_t capacity) :
        capacity_(capacity),
        lirs_max_(capacity > 0 ? capacity - std::max<std::size_t>(1, capacity / 100) : 0)
    {}

    std::size_t max_capacity() const { return capacity_ ; }

    bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page)
    {
        if (max_capacity() == 0)
            return false;

        auto hit_stack = hash_stack_.find(key);
        bool in_stack = hit_stack != hash_stack_.end();
        auto hit_queue = hash_queue_.find(key);
        bool in_queue = hit_queue != hash_queue_.end();

        if (in_stack && hit_stack->second->status == BlockStatus::LIR)
        {
            bool was_at_bottom = (hit_stack->second == std::prev(stack_.end()));
            stack_.splice(stack_.begin(), stack_, hit_stack->second);

            if (was_at_bottom)
                prune_stack();

            return true;
        }

        if (in_queue)
        {
            if (in_stack)
            {
                stack_.splice(stack_.begin(), stack_, hit_stack->second);
                hit_stack->second->status = BlockStatus::LIR;
                ++lir_count_;

                queue_.erase(hit_queue->second);
                hash_queue_.erase(key);

                prune_stack();
                demote_lir_bottom();
                prune_stack();
            }
            else
            {
                queue_.splice(queue_.begin(), queue_, hit_queue->second);
                stack_.emplace_front(key, hit_queue->second->data, BlockStatus::HIR);
                hash_stack_.emplace(key, stack_.begin());
            }

            return true;
        }

        T page = slow_get_page(key);

        if (!is_full())
        {
            cache_.emplace_front(std::move(page));

            if (lir_count_ < lirs_max_)
            {
                stack_.emplace_front(key, cache_.begin(), BlockStatus::LIR);
                hash_stack_.emplace(key, stack_.begin());
                ++lir_count_;
            }
            else
            {
                stack_.emplace_front(key, cache_.begin(), BlockStatus::HIR);
                hash_stack_.emplace(key, stack_.begin());
                queue_.emplace_front(key, cache_.begin());
                hash_queue_.emplace(key, queue_.begin());
            }

            return false;
        }

        evict_lru_hir();
        cache_.emplace_front(std::move(page));

        if (in_stack)
        {
            stack_.splice(stack_.begin(), stack_, hit_stack->second);
            hit_stack->second->data = cache_.begin();
            hit_stack->second->status = BlockStatus::LIR;
            ++lir_count_;

            prune_stack();
            demote_lir_bottom();
            prune_stack();
        }
        else
        {
            stack_.emplace_front(key, cache_.begin(), BlockStatus::HIR);
            hash_stack_.emplace(key, stack_.begin());
            queue_.emplace_front(key, cache_.begin());
            hash_queue_.emplace(key, queue_.begin());
        }

        return false;
    }
};

} // namespace caches

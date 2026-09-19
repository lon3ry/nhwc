#pragma once

#include <functional>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <list>
#include <unordered_map>

namespace caches
{

template <typename T, typename KeyT = int> class LIRSCache
{
public:
    explicit LIRSCache(std::size_t capacity) :
        capacity_(capacity),
        lirs_max_(capacity - std::max<std::size_t>(1, capacity / 100))
    {}
    bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page);

private:
    std::size_t capacity_, lirs_max_;

    std::list<T> cache_;

    enum class BlockStatus { LIR, HIR };
    using CacheIt = typename std::list<T>::iterator;
    struct Record
    {
        KeyT key;
        CacheIt data;
        BlockStatus status;
    };
    using StackIt = typename std::list<Record>::iterator;
    using QueueIt = typename std::list<Record>::iterator;

    std::list<Record> stack_;
    std::list<Record> queue_;
    std::unordered_map<KeyT, StackIt> hash_stack_;
    std::unordered_map<KeyT, QueueIt> hash_queue_;

    std::size_t lir_count_ = 0;

    void prune_stack();

    void demote_lir_bottom();

    BlockStatus get_block_status(KeyT key) const;

    bool is_hir(KeyT key) const { return get_block_status(key) == BlockStatus::HIR; }
    bool is_lir(KeyT key) const { return get_block_status(key) == BlockStatus::LIR; }
    bool is_resident(KeyT key) const;

    bool is_full() const { return cache_.size() >= capacity_; }

    void free_space();
};

template <typename T, typename KeyT>
void LIRSCache<T, KeyT>::prune_stack()
{
    while (!stack_.empty() && stack_.back().status != BlockStatus::LIR)
    {
        Record bottom = stack_.back();
        stack_.pop_back();
        hash_stack_.erase(bottom.key);
    }
}

template <typename T, typename KeyT>
void LIRSCache<T, KeyT>::demote_lir_bottom()
{
    if (stack_.empty() || stack_.back().status != BlockStatus::LIR)
        return;

    Record bottom = stack_.back();
    stack_.pop_back();
    hash_stack_.erase(bottom.key);

    bottom.status = BlockStatus::HIR;
    queue_.emplace_front(bottom.key, bottom.data, BlockStatus::HIR);
    hash_queue_.emplace(bottom.key, queue_.begin());

    --lir_count_;
}

template <typename T, typename KeyT>
LIRSCache<T, KeyT>::BlockStatus LIRSCache<T, KeyT>::get_block_status(KeyT key) const
{
    auto hit_stack = hash_stack_.find(key);
    if (hit_stack != hash_stack_.end())
        return hit_stack->second->status;

    auto hit_queue = hash_queue_.find(key);
    if (hit_queue != hash_queue_.end())
        return hit_queue->second->status;

    return BlockStatus::HIR;
}

template <typename T, typename KeyT>
bool LIRSCache<T, KeyT>::is_resident(KeyT key) const
{
    auto hit_stack = hash_stack_.find(key);
    if (hit_stack != hash_stack_.end())
        return hit_stack->second->data != cache_.end();

    auto hit_queue = hash_queue_.find(key);
    if (hit_queue != hash_queue_.end())
        return hit_queue->second->data != cache_.end();

    return false;
}

template <typename T, typename KeyT>
void LIRSCache<T, KeyT>::free_space()
{
    if (!is_full())
        return;

    auto victim = queue_.back();
    auto hit_stack = hash_stack_.find(victim.key);
    if (hit_stack != hash_stack_.end())
        hit_stack->second->data = cache_.end();

    cache_.erase(victim.data);
    hash_queue_.erase(victim.key);
    queue_.pop_back();
}

template <typename T, typename KeyT>
bool LIRSCache<T, KeyT>::lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page)
{
    auto hit_stack = hash_stack_.find(key);
    auto hit_queue = hash_queue_.find(key);

    bool was_in_stack = hit_stack != hash_stack_.end();
    bool was_in_queue = hit_queue != hash_queue_.end();

    if (is_lir(key))
    {
        bool was_at_bottom = (hit_stack->second == std::prev(stack_.end()));
        stack_.splice(stack_.begin(), stack_, hit_stack->second);

        if (was_at_bottom)
            prune_stack();

        return true;
    }

    if (is_resident(key))
    {
        if (was_in_stack)
        {
            stack_.splice(stack_.begin(), stack_, hit_stack->second);

            hit_stack->second->status = BlockStatus::LIR;
            ++lir_count_;

            if (was_in_queue)
            {
                queue_.erase(hit_queue->second);
                hash_queue_.erase(key);
            }

            demote_lir_bottom();
            prune_stack();
            return true;
        }

        queue_.splice(queue_.begin(), queue_, hit_queue->second);
        return true;
    }

    T page = slow_get_page(key);

    if (lir_count_ < lirs_max_)
    {
        cache_.emplace_front(std::move(page));
        stack_.emplace_front(key, cache_.begin(), BlockStatus::LIR);
        hash_stack_.emplace(key, stack_.begin());
        ++lir_count_;
        return false;
    }

    free_space();

    if (was_in_stack)
    {
        cache_.emplace_front(std::move(page));
        hit_stack->second->data = cache_.begin();
        hit_stack->second->status = BlockStatus::LIR;
        ++lir_count_;
        stack_.splice(stack_.begin(), stack_, hit_stack->second);

        demote_lir_bottom();
        prune_stack();
    }
    else
    {
        cache_.emplace_front(std::move(page));
        stack_.emplace_front(key, cache_.begin(), BlockStatus::HIR);
        hash_stack_.emplace(key, stack_.begin());

        queue_.emplace_front(key, cache_.begin(), BlockStatus::HIR);
        hash_queue_.emplace(key, queue_.begin());
    }

    return false;
}

} // namespace caches

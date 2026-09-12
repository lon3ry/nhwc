#include <cstddef>

#include <list>
#include <unordered_map>
#include <cstddef>

#include "lru-list.hpp"
#include "ghost-lru-list.hpp"

namespace caches
{

template <typename T, typename KeyT = int> class ARCCache
{
    const std::size_t size_;

    LRUList<T, KeyT> recents_;           // list of once seen pages, T1
    LRUList<T, KeyT> frequenters_;       // list of twice seen pages, T2
    GhostLRUCache<KeyT> old_recents_;     // ghost cache for T1, named B1
    GhostLRUCache<KeyT> old_frequenters_; // ghost cache for T2, named B2

    std::size_t p_ = 0;

    void replace(KeyT key)
    {
        if (!recents_.empty() && (recents_.size() > p_ || (old_frequenters_.has(key) &&
            p_ == recents_.size())))
        {
            auto page = recents_.pop_last_recently_used();
            old_recents_.insert(page);
        }
        else
        {
            auto page = frequenters_.pop_last_recently_used();
            old_frequenters_.insert(page);
        }
    }

public:
    explicit ARCCache(std::size_t size) : size_(size) {}

    template <typename F> bool lookup_update(KeyT key, F slow_get_page)
    {
        bool hit_recents = recents_.lookup(key);
        bool hit_frequenters = frequenters_.lookup(key);

        if (hit_recents)
        {
                auto page = recents_.pop_most_recently_used();
                frequenters_.insert(key, page);
                return true;
        }

        if (hit_frequenters)
            return true;

        bool hit_old_recents = old_recents_.lookup(key);
        if (hit_old_recents)
        {
            std::size_t delta1 = old_recents_.size() >= old_frequenters_.size() ? 1 :
                                 old_frequenters_.size() / old_recents_.size();
            p_ = std::min(p_ + delta1, size_);
            replace(key);
            old_recents_.pop_most_recently_used();
            auto page = slow_get_page(key);
            frequenters_.insert(key, page);
            return false;
        }

        bool hit_old_frequenters = old_frequenters_.lookup(key);
        if (hit_old_frequenters)
        {
            std::size_t delta2 = old_frequenters_.size() >= old_recents_.size() ? 1 :
                                 old_recents_.size() / old_frequenters_.size();
            p_ = std::max(p_ - delta2, size_);
            replace(key);
            old_frequenters_.pop_most_recently_used();
            auto page = slow_get_page(key);
            frequenters_.insert(key, page);
            return false;
        }

        if (size_ > recents_.size() + old_recents_.size())
        {
            if (recents_.size() < size_)
            {
                old_recents_.pop_most_recently_used();
                replace(key);
            }
            else
            {
                recents_.pop_most_recently_used();
            }
        }
        else
        {
            auto current_size = recents_.size() + frequenters_.size() + old_recents_.size() +
                                old_frequenters_.size();
            if (current_size >= size_)
            {
                if (current_size == size_)
                    old_frequenters_.pop_last_recently_used();
                replace(key);
            }
            auto page = slow_get_page(key);
            recents_.insert(key, page);
        }
        return false;
    }
};

} // namespace caches

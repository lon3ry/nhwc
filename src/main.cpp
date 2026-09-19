#include <iostream>

#include "two_queue_cache.hpp"

using Page = int;
using PageId = int;

int main()
{
    int cache_size, data_len; // std::size_t cache_size instead of int

    // TODO: add input check
    std::cin >> cache_size >> data_len;

    caches::TwoQueueCache<Page, PageId> cache{static_cast<std::size_t>(cache_size)}; // why cast?
    auto load = [](PageId key) { return key; };

    int hits = 0;
    for (int i = 0; i < data_len; i++)
    {
        PageId key;
        std::cin >> key;

        bool hit = cache.lookup_update(key, load);
        if (hit)
            ++hits;
    }
    std::cout << hits << std::endl;
}

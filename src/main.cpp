#include <iostream>

#include "LFU_cache.hpp"

using Page = int;
using PageId = int;

int main()
{
    int cache_size, data_len;

    // TODO: add input check
    std::cin >> cache_size >> data_len;

    LFUCache<Page, PageId> cache{static_cast<std::size_t>(cache_size)};
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

#include <iostream>

#include "belady_cache.hpp"
#include "util.hpp"

using namespace caches;

using Page = long long;
using PageId = long long;

int main()
{
    long long cache_size, data_len;
    if (!read_integer(cache_size) || !read_integer(data_len) || can_not_be_valid_size_t(cache_size)
        || can_not_be_valid_size_t(data_len))
    {
        std::cerr << "Expected nonnegative cache and data size.\n";
        return 1;
    }

    BeladyCache<Page, PageId> cache(cache_size);

    auto load = [](PageId key) { return key; };

    std::vector<PageId> requests(data_len);

    for (int i = 0; i < data_len; i++)
    {
        PageId key;

        auto read_ok = read_integer(key);
        if (!read_ok || can_not_be_valid_size_t(key))
        {
            std::cerr << "Expected nonnegative page key.\n";
            return 1;
        }

        requests[i] = key;
    }
    std::cout << cache.calculate_hits(requests) << std::endl;
}

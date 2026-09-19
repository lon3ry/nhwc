#include <iostream>

#include "multi_level_cache.hpp"
#include "config.hpp"

using namespace caches;
using Page = long long;
using PageId = long long;

bool read_integer(long long& value);
bool can_be_valid_size_t(long long value);

int main()
{
    auto levels = parse_cache_levels_algorithms();
    for (auto& level : levels)
    {
        long long n;
        auto read_ok = read_integer(n);
        if (!read_ok || can_be_valid_size_t(n))
        {
            std::cerr << "Expected nonnegative cache size.\n";
            return 1;
        }
        level.capacity = n;
    }

    MultiLevelCache<Page, PageId> cache(levels);

    long long data_len;
    auto read_ok = read_integer(data_len);
    if (!read_ok || can_be_valid_size_t(data_len))
    {
        std::cerr << "Expected nonnegative cache size.\n";
        return 1;
    }

    auto load = [](PageId key) { return key; };

    unsigned int hits = 0;
    for (int i = 0; i < data_len; i++)
    {
        PageId key;

        auto read_ok = read_integer(key);
        if (!read_ok || can_be_valid_size_t(key))
        {
            std::cerr << "Expected nonnegative page key.\n";
            return 1;
        }

        bool hit = cache.lookup_update(key, load);
        if (hit)
            ++hits;
    }
    std::cout << hits << std::endl;
}

bool read_integer(long long& value)
{
    if (!(std::cin >> value))
        return false;
    // Reject tokens such as "12x"; reaching EOF after a number is valid.
    const auto next = std::cin.peek();
    return !std::cin.bad() && (next == std::char_traits<char>::eof() ||
           std::isspace(static_cast<unsigned char>(next)));
}

bool can_be_valid_size_t(long long value)
{
    return value < 0 ||
           static_cast<unsigned long long>(value) > std::numeric_limits<std::size_t>::max();
}

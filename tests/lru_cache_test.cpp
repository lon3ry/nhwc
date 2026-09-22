#include <gtest/gtest.h>
#include <vector>

#include "lru_cache.hpp"

TEST(LRUCacheTest, CacheHitsTest)
{
    struct TestCase
    {
        int cache_size;
        std::vector<int> values;
        int hits;
    };

    const std::vector<TestCase> test_cases = {
        { 2, { 1, 2, 1, 2, 1, 2 }, 4 },
        { 0, { 1, 1, 1 }, 0 },
        { 1, { 7 }, 0 },
        { 1, { 7, 7, 7, 7 }, 3 },
        { 1, { 1, 2, 1, 2 }, 0 },
        { 10, { 1, 2, 3, 1, 2, 3 }, 3 },
        { 2, { 1, 2, 3, 4, 5 }, 0 },
        { 2, { 1, 2, 3, 1, 2, 3 }, 0 },
        { 2, { 1, 2, 1, 3, 1, 2 }, 2 },
        { 3, { 1, 2, 3, 2, 4, 2, 3, 1 }, 3 },
        { 3, { 0, -1, -2, 0, -1, -2 }, 3 },

        // Empty requests, zero capacity, and a cache with one slot.
        { 0, {}, 0 },
        { 0, { 1 }, 0 },
        { 0, { 1, 1, 2, 1, 2 }, 0 },
        { 1, {}, 0 },
        { 4, {}, 0 },
        { 1, { 42 }, 0 },
        { 8, { 42 }, 0 },
        { 1, { 7, 7, 7, 7, 7, 7 }, 5 },
        { 4, { 7, 7, 7, 7, 7, 7 }, 5 },
        { 1, { 1, 2, 1, 2, 1 }, 0 },
        { 1, { 1, 1, 2, 2, 1, 1 }, 3 },
        { 1, { 1, 1, 2, 2, 3, 3, 1, 1, 2, 2, 3, 3 }, 6 },

        // Cache filling, repeated scans, and capacity boundaries.
        { 2, { 1, 2, 1, 2, 1, 2 }, 4 },
        { 3, { 1, 2, 3, 1, 2, 3, 1, 2, 3 }, 6 },
        { 8, { 1, 2, 3, 1, 2, 3, 1, 2, 3 }, 6 },
        { 1000000, { 1, 2, 3, 4, 5, 1, 2, 3, 4, 5 }, 5 },
        { 3, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }, 0 },
        { 2, { 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3 }, 0 },
        { 3, { 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4 }, 0 },
        { 4, { 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4 }, 8 },
        { 3, { 1, 2, 3, 4, 3, 2, 1 }, 2 },

        // Zero, negative, and extreme int key values.
        { 0, { 0, -1, 0, -1 }, 0 },
        { 1, { 0, 0, -1, -1, 0, 0 }, 3 },
        { 3, { -1, 0, 1, -1, 0, 1, -1, 0, 1 }, 6 },
        { 2, { -3, -2, -3, -1, -2, -3, -1, -2 }, 1 },
        { 2, { -2147483648, 2147483647, -2147483648, 2147483647, 0, -2147483648, 2147483647 }, 2 },
        { 4, { -2147483648, -1, 0, 2147483647, -2147483648, -1, 0, 2147483647, -2147483648, -1, 0,
               2147483647
        }, 8 },

        // Hits move the oldest and middle entries to the MRU position.
        { 2, { 1, 2, 1, 3, 1, 2 }, 2 },
        { 3, { 1, 2, 3, 2, 4, 2, 3, 1 }, 3 },
        { 3, { 1, 2, 3, 1, 4, 1, 2, 3 }, 2 },
        { 3, { 1, 2, 3, 2, 4, 2, 1, 3 }, 2 },
        { 3, { 1, 2, 3, 3, 4, 3, 1, 2 }, 2 },
        { 2, { 1, 2, 2, 2, 3, 2, 1 }, 3 },

        // Recency determines eviction even when an older key has more hits.
        { 2, { 1, 1, 1, 1, 2, 3, 1 }, 3 },
        { 2, { 1, 1, 1, 1, 2, 3, 2 }, 4 },
        { 3, { 1, 1, 1, 1, 2, 2, 3, 4, 1, 2, 3 }, 4 },
        { 3, { 1, 2, 1, 2, 1, 2, 3, 4, 5, 1, 2 }, 4 },

        // Evicted keys reenter as new entries during repeated scans.
        { 2, { 1, 2, 3, 1, 1, 4, 1, 2 }, 2 },
        { 3, { 1, 2, 3, 4, 1, 2, 3, 4, 4, 3, 2, 1 }, 3 },
        { 4, { 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1, 2, 3, 4 }, 6 },
        { 4, { 1, 2, 3, 4, 1, 2, 3, 4, 5, 6, 7, 8, 5, 6, 7, 8, 1, 2, 3, 4 }, 8 },
        { 5, { 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 8, 7, 6, 5, 4, 3, 2, 1, 8, 7, 6, 5,
               4, 3, 2, 1 }, 5 },
    };

    auto load = [](int key) { return key; };

    for (auto test : test_cases)
    {
        SCOPED_TRACE("test vector: " + ::testing::PrintToString(test.values));

        caches::LRUCache<int, int> cache(test.cache_size);
        int hits = 0;
        for (auto v : test.values)
        {
            auto hit = cache.lookup_update(v, load);
            if (hit)
                hits++;
        }
        EXPECT_EQ(hits, test.hits);
    }
}

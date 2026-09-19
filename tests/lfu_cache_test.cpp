#include <gtest/gtest.h>
#include <vector>

#include "lfu_cache.hpp"

TEST(LFUCacheTest, CacheHitsTest)
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
    };

    auto load = [](int key) { return key; };

    for (auto test : test_cases)
    {
        SCOPED_TRACE("test vector: " + ::testing::PrintToString(test.values));

        LFUCache<int, int> cache(test.cache_size);
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

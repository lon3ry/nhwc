#include <gtest/gtest.h>
#include <vector>

#include "lirs_cache.hpp"

TEST(LIRSCacheTest, CacheHitsTest)
{
    struct TestCase
    {
        int cache_size;
        std::vector<int> values;
        int hits;
    };

    const std::vector<TestCase> test_cases = {
        {4, {1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6}, 3},
        {5, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 5}, 4},
        {3, {1, 2, 3, 4, 5, 6, 7, 1, 2, 1, 2}, 4},
        {4, {1, 2, 3, 4, 5, 1, 6, 1, 7, 1, 8, 1, 9, 1, 10}, 5},
        {5, {1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5}, 10},
        {4, {1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4}, 3},
        {4, {1, 5, 2, 6, 3, 7, 4, 8, 1, 5, 2, 6}, 3},
        {3, {1, 2, 3, 4, 5, 6, 7, 8, 9}, 0},
        {4, {1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5}, 10},
        {3, {1, 2, 3, 1, 4, 1, 5, 1, 6, 1}, 4}
    };

    auto load = [](int key) { return key; };

    for (auto test : test_cases)
    {
        SCOPED_TRACE("test vector: " + ::testing::PrintToString(test.values));

        caches::LIRSCache<int, int> cache(test.cache_size);
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
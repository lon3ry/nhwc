#include <gtest/gtest.h>
#include <vector>

#include "two_queue.hpp"

TEST(TwoQueueCacheTest, CacheHitsTest)
{
    struct TestCase
    {
        int cache_size;
        std::vector<int> values;
        int hits;
    };

    const std::vector<TestCase> test_cases = {
        { 2, { 5, 5, 5, 5, 5 }, 4 },
        { 2, { 1, 2, 1, 2, 1, 2 }, 4 },
        { 3, { 1, 2, 3, 4, 1, 2, 1, 2 }, 2 },
        { 4, { 1, 2, 3, 4, 5, 1, 6, 2, 3, 4, 3, 4 }, 2 },
        { 4, { 1, 2, 3, 4, 5, 6, 7, 1, 1 }, 1 },
        { 6, { 1, 2, 3, 4, 5, 6, 100, 200, 7, 8, 9, 10, 11, 12, 100, 200, 1000, 1001, 1002, 1003,
               1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017,
               1018, 1019, 1020, 1021, 1022, 1023, 1024, 100, 200 }, 2 },
        { 6, { 1, 101, 2, 102, 3, 103, 1, 104, 2, 105, 3, 106, 1, 107, 2, 108, 3, 109, 1, 110, 2,
               111, 3, 112, 1, 113, 2, 114, 3, 115, 1, 116, 2, 117, 3, 118, 1, 119, 2, 120, 3,
               121, 1, 122, 2, 123, 3, 124, 1, 125, 2, 126, 3, 127, 1, 128, 2, 129, 3, 130 }, 24 }
    };

    auto load = [](int key) { return key; };

    for (auto test : test_cases)
    {
        caches::TwoQueueCache<int, int> cache(test.cache_size);
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

#include <gtest/gtest.h>
#include <vector>

#include "belady_cache.hpp"

TEST(BeladyCacheTest, CacheHitsTest)
{
    struct TestCase
    {
        int cache_size;
        std::vector<int> values;
        int hits;
    };

    const std::vector<TestCase> test_cases = {
        { 2,  { 1, 2, 1, 2, 1, 2 }, 4 },
        { 0,  { 1, 1, 1 }, 0 },
        { 1,  { 7 }, 0 },
        { 1,  { 7, 7, 7, 7 }, 3 },
        { 1,  { 1, 2, 1, 2 }, 0 },
        { 1,  { 1, 2, 1, 2, 1, 2 }, 0 },
        { 10, { 1, 2, 3, 1, 2, 3 }, 3 },
        { 2,  { 1, 2, 3, 4, 5 }, 0 },
        { 2,  { 1, 2, 3, 1, 2, 3 }, 2 },
        { 2,  { 1, 2, 3, 1, 3, 2 }, 2 },
        { 2,  { 1, 2, 1, 3, 1, 2 }, 2 },
        { 3,  { 1, 2, 3, 2, 4, 2, 3, 1 }, 3 },
        { 3,  { 0, -1, -2, 0, -1, -2 }, 3 },
        { 2,  { 1, 1, 2, 3, 3, 1 }, 3 },
        { 2,  { 1, 1, 2, 3, 1, 2 }, 2 },
        { 3,  { 1, 2, 3, 1, 2, 4, 1, 2, 5, 1, 2 }, 6 },
        { 3,  { 1, 1, 2, 2, 3, 3, 3, 2, 1, 4, 1, 2, 3 }, 8 },
        { 1,  { 1, 2, 2, 3, 3, 3, 4, 1 }, 3 },
        { 4,  { 1, 2, 3, 4, 1, 2, 5, 3, 4, 3, 1, 2 }, 6 },
        { 2,  { 1, 2, 3, 3, 3, 3, 1, 2 }, 4 },
        { 3,  { 1, 2, 3, 4, 5, 1, 2, 3, 4, 5 }, 3 },
        { 4,  { 1, 2, 3, 4, 1, 2, 3, 4 }, 4 },
        { 2,  { 0, -1, -2, 0, -1, -2 }, 2 },
        { 3,  { 1, 2, 3, 4, 5, 6 }, 0 },
        { 2,  { 1, 2, 1, 3, 4, 1, 2 }, 2 },
    };

    for (auto test : test_cases)
    {
        SCOPED_TRACE("test vector: " + ::testing::PrintToString(test.values));

        BeladyCache<int, int> cache(test.cache_size);
        int hits = static_cast<int>(cache.calculate_hits(test.values));

        EXPECT_EQ(hits, test.hits);
    }
}
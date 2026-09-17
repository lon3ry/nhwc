#include <iostream>
#include "LFU_cache.hpp"


int slow_get_page(int key);


int main()
{
    LFUCache<int, int> cache(2);

    int test_data[] = {1, 2, 1, 2, 3, 1, 2};

    for(int i : test_data)
    {
        VERBOSE(std::cout << "Current data request: " << i << std::endl;)
        cache.lookup_update(i, slow_get_page);
        VERBOSE(cache.cache_dump();)
        std::cout << "-----------" << std::endl;
    }

    return 0;
}


int slow_get_page(int key)
{
    return key;
}
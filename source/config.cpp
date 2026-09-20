#include <fstream>
#include <iostream>
#include <cstdlib>

#include "multi_level_cache.hpp"
#include "config.hpp"

std::list<caches::CacheLevel> parse_cache_levels_algorithms()
{
    std::ifstream file(config_file);

    if (!file)
    {
        std::cerr << "Unable to open config file '" << config_file << "'." << std::endl;
        std::exit(1);
    }

    std::list<caches::CacheLevel> levels;
    std::size_t cache_levels;

    file >> cache_levels;
    for (int i = 0; i < cache_levels; i++)
    {
        std::string level_algorithm;

        file >> level_algorithm;
        caches::CacheLevel level = {
            .type = caches::string_to_cache_type(level_algorithm),
            .capacity = 0
        };
        levels.emplace_back(level);
    }

    return levels;
}

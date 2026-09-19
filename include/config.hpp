#pragma once

#include "multi_level_cache.hpp"

constexpr auto config_file = "config.txt";
std::list<caches::CacheLevel> parse_cache_levels_algorithms();

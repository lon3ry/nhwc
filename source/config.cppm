module;

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <vector>
#include <print>
#include <ranges>

export module config;

import multi_level_cache;

namespace caches::config {

export std::vector<caches::CacheLevel>
parse_cache_levels_algorithms(const std::filesystem::path& config_path) {
  std::ifstream file{config_path};

  if (!file) {
    std::println("Unable to open config file {}", config_path.string());
    std::exit(1);
  }

  std::vector<caches::CacheLevel> levels;
  std::size_t cache_levels;

  // TODO: see istream iterators with range
  file >> cache_levels;
  for (auto _ : std::views::iota(0uz, cache_levels)) {
    std::string level_algorithm;
    file >> level_algorithm;
    levels.emplace_back(caches::string_to_cache_type(level_algorithm), 0);
  }

  return levels;
}

}  // namespace caches::config

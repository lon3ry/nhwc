#include <iostream>
#include <cstddef>
#include <print>
#include <ranges>

import cli11;

import multi_level_cache;
import config;
import util;

using PageId = unsigned int;
using Page = unsigned int;

int main(int argc, char* argv[]) {
  CLI::App app{"NHWC"};

  std::string config_path;
  app.add_option("-c,--config", config_path, "The config path")->required();

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError& e) {
    return app.exit(e);
  }

  auto levels = caches::config::parse_cache_levels_algorithms(config_path);
  for (auto& level : levels) {
    auto result = util::read_integer<std::size_t>();
    if (!result) return 1;
    level.capacity = result.value();
  }

  caches::MultiLevelCache<PageId, Page> cache{levels};

  auto result = util::read_integer<std::size_t>();
  if (!result) return 1;

  auto data_len = result.value();

  auto load = [](PageId key) { return key; };

  unsigned int hits = 0;
  for (auto _ : std::views::iota(0uz, data_len)) {
    auto key = util::read_integer<PageId>();
    if (!key) return 1;

    bool hit = cache.lookup_update(key.value(), load);
    if (hit) {
      ++hits;
    }
  }

  if (std::cin.peek() != std::char_traits<char>::eof()) {
    std::println("warning: too much arguments, only {} requests were handled.", data_len);
  }

  std::println("{}", hits);
}

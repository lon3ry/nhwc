#include <print>
#include <iostream>
#include <ranges>

import belady_cache;
import util;

using Page = unsigned int;
using PageId = unsigned int;

int main() {
  auto cache_size = util::read_integer<std::size_t>();
  if (!cache_size) return 1;

  auto data_len = util::read_integer<std::size_t>();
  if (!data_len) return 1;

  caches::BeladyCache<PageId, Page> cache(cache_size.value());

  std::vector<PageId> requests(data_len.value());
  for (const auto& i : std::views::iota(0uz, data_len.value())) {
    auto key = util::read_integer<PageId>();
    if (!key) return 1;
    requests[i] = key.value();
  }

  if (std::cin.peek() != std::char_traits<char>::eof()) {
    std::println("warning: too much arguments, only {} requests were handled.", data_len.value());
  }

  auto load = [](PageId key) { return key; };

  std::println("{}", cache.calculate_hits(requests, load));
}

#include <string_view>
#include <stdexcept>

#include "multi_level_cache.hpp"

namespace caches
{

CacheType string_to_cache_type(const std::string_view str)
{
    if (str == "LRU")
        return CacheType::LRU;
    if (str == "ARC")
        return CacheType::ARC;
    if (str == "2Q")
        return CacheType::TWO_QUEUE;
    if (str == "LFU")
        return CacheType::LFU;
    throw std::invalid_argument("unknown cache type: " + std::string(str));
}

} // namespace caches

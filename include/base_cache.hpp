#pragma once

#include <functional>

namespace caches
{

template <typename T, typename KeyT = int> class BaseCache
{
public:
    virtual bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page) = 0;

    virtual ~BaseCache() = default;
};

} // namespace caches

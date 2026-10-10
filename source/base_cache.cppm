module;

#include <functional>
#include <cstddef>

export module base_cache;

namespace caches {

export template <typename Key, typename Value>
class BaseCache {
public:
  explicit BaseCache(std::size_t capacity) : capacity_(capacity) {}
  virtual ~BaseCache() = default;

  std::size_t max_capacity() const { return capacity_; }

  bool lookup_update(const Key& key, std::function<Value(Key)> slow_get_page) {
    if (max_capacity() == 0) {
      return false;
    }

    return do_lookup_update(key, slow_get_page);
  }

private:
  std::size_t capacity_;

  virtual bool do_lookup_update(const Key& key, std::function<Value(Key)> slow_get_page) = 0;
};

}  // namespace caches

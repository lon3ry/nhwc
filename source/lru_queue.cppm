module;

#include <cstddef>
#include <list>
#include <optional>
#include <unordered_map>

export module lru_queue;

namespace caches {

template <typename Key, typename QueueItem>
class BaseLRUQueue {
public:
  virtual ~BaseLRUQueue() = default;

  std::size_t size() const { return cache_.size(); }
  bool empty() const { return size() == 0; }
  bool has(const Key& key) const { return hash_.contains(key); }

  std::optional<QueueItem> pop_most_recently_used() { return pop(cache_.begin()); }
  std::optional<QueueItem> pop_last_recently_used() { return pop(std::prev(cache_.end())); }

  bool lookup(const Key& key) {
    auto hit = hash_.find(key);
    if (hit != hash_.end()) {
      auto eltit = hit->second;
      cache_.splice(cache_.begin(), cache_, eltit);
      return true;
    }
    return false;
  }

  void insert(const QueueItem& item) {
    cache_.emplace_front(item);
    hash_.emplace(get_key(item), cache_.begin());
  }

  void erase(const Key& key) {
    auto hit = hash_.find(key);
    if (hit == hash_.end()) return;
    auto eltit = hit->second;
    cache_.erase(eltit);
    hash_.erase(hit);
  }

private:
  std::list<QueueItem> cache_;

  using QueueIt = typename std::list<QueueItem>::iterator;
  std::unordered_map<Key, QueueIt> hash_;

  static auto get_key(const QueueItem& item) {
    if constexpr (std::is_same_v<Key, QueueItem>) {
      return item;
    } else {
      return item.key;
    }
  }

  std::optional<QueueItem> pop(std::list<QueueItem>::iterator it) {
    if (empty()) return std::nullopt;

    hash_.erase(get_key(*it));
    auto item = *it;
    cache_.erase(it);

    return item;
  }
};


template <typename Key, typename Value>
struct LRUQueueItem { Key key; Value page; };

export template <typename Key, typename Value>
class LRUQueue final : public BaseLRUQueue<Key, LRUQueueItem<Key, Value>> {};

export template <typename Key>
class GhostLRUQueue final : public BaseLRUQueue<Key, Key> {};

} // namespace caches

module;

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <list>
#include <unordered_map>

export module lirs_cache;

import base_cache;

namespace caches {

export template <typename Key, typename Value>
class LIRSCache final : public BaseCache<Key, Value> {
public:
  explicit LIRSCache(std::size_t capacity) :
    BaseCache<Key, Value>(capacity),
    lir_max_(capacity > 0 ? capacity - std::max<std::size_t>(1, capacity / 100) : 0) {}

  using BaseCache<Key, Value>::max_capacity;

  bool is_full() const { return lir_count_ + queue_.size() >= max_capacity(); }

private:
  std::size_t lir_max_;
  std::size_t lir_count_ = 0;

  std::list<Value> data_;
  enum class BlockStatus : bool { kLIR, kHIR };
  using CacheIt = typename std::list<Value>::iterator;

  struct Record {
    Key key;
    CacheIt data;
    BlockStatus status;

    Record(Key k, CacheIt d, BlockStatus s) : key(k), data(d), status(s) {}
  };
  using RecordIt = typename std::list<Record>::iterator;

  std::list<Record> stack_;
  std::unordered_map<Key, RecordIt> hash_stack_;

  std::list<Record> queue_;
  std::unordered_map<Key, RecordIt> hash_queue_;

  RecordIt find_in_stack(const Key& key) {
    auto hit = hash_stack_.find(key);
    return hit == hash_stack_.end() ? stack_.end() : hit->second;
  }

  RecordIt find_in_queue(Key key) {
    auto hit = hash_queue_.find(key);
    return hit == hash_queue_.end() ? queue_.end() : hit->second;
  }

  CacheIt add_to_data(const Value& page) {
    data_.emplace_front(page);
    return data_.begin();
  }

  void add_to_stack_top(const Key& key, CacheIt data, BlockStatus status) {
    stack_.emplace_front(key, data, status);
    hash_stack_.emplace(key, stack_.begin());
  }

  void add_to_queue_top(const Key& key, CacheIt data) {
    queue_.emplace_front(key, data, BlockStatus::kHIR);
    hash_queue_.emplace(key, queue_.begin());
  }

  bool move_to_stack_top(const Key& key) {
    RecordIt stack_it = find_in_stack(key);
    if (stack_it == stack_.end()) {
      return false;
    }

    stack_.splice(stack_.begin(), stack_, stack_it);
    return true;
  }

  bool move_to_queue_top(const Key& key) {
    RecordIt queue_it = find_in_queue(key);
    if (queue_it == queue_.end()) {
      return false;
    }

    queue_.splice(queue_.begin(), queue_, queue_it);
    return true;
  }

  bool remove_from_queue(const Key& key) {
    RecordIt queue_it = find_in_queue(key);
    if (queue_it == queue_.end()) {
      return false;
    }

    hash_queue_.erase(key);
    queue_.erase(queue_it);
    return true;
  }

  void prune_stack() {
    while (!stack_.empty() && stack_.back().status == BlockStatus::kHIR) {
      hash_stack_.erase(stack_.back().key);
      stack_.pop_back();
    }
  }

  void demote_to_hir() {
    if (stack_.empty()) {
      return;
    }

    RecordIt bottom = std::prev(stack_.end());
    bottom->status = BlockStatus::kHIR;
    hash_stack_.erase(bottom->key);
    queue_.splice(queue_.begin(), stack_, bottom);
    hash_queue_.emplace(bottom->key, bottom);
    --lir_count_;
  }

  void evict_lru_hir() {
    if (queue_.empty()) {
      return;
    }

    Key key = std::move(queue_.back().key);
    CacheIt data = queue_.back().data;
    hash_queue_.erase(key);
    queue_.pop_back();

    RecordIt stack_it = find_in_stack(key);
    if (stack_it != stack_.end()) {
      stack_it->data = data_.end();
    }
    data_.erase(data);
  }

  bool lookup_lir(const Key& key) {
    RecordIt stack_it = find_in_stack(key);
    if (stack_it == stack_.end() || stack_it->status != BlockStatus::kLIR) {
      return false;
    }

    move_to_stack_top(key);
    prune_stack();
    return true;
  }

  bool lookup_hir(const Key& key) {
    RecordIt queue_it = find_in_queue(key);
    if (queue_it == queue_.end() || queue_it->status != BlockStatus::kHIR) {
      return false;
    }

    RecordIt stack_it = find_in_stack(key);
    if (stack_it != stack_.end()) {
      move_to_stack_top(key);
      stack_it->status = BlockStatus::kLIR;
      ++lir_count_;

      remove_from_queue(key);
      demote_to_hir();
      prune_stack();
    } else {
      move_to_queue_top(key);
      add_to_stack_top(key, queue_it->data, BlockStatus::kHIR);
      prune_stack();
    }
    return true;
  }

  void insert(const Key& key, const Value& page) {
    if (is_full()) {
      evict_lru_hir();
    }

    CacheIt data = add_to_data(std::move(page));

    RecordIt stack_it = find_in_stack(key);
    if (stack_it != stack_.end()) {
      move_to_stack_top(key);
      stack_it->data = data;
      stack_it->status = BlockStatus::kLIR;
      ++lir_count_;
      demote_to_hir();
      prune_stack();
      return;
    }

    if (lir_count_ < lir_max_) {
      add_to_stack_top(key, data, BlockStatus::kLIR);
      ++lir_count_;
      return;
    }

    add_to_stack_top(key, data, BlockStatus::kHIR);
    prune_stack();
    add_to_queue_top(key, data);
  }

  bool do_lookup_update(const Key& key, std::function<Value(Key)> slow_get_page) override {
    if (lookup_lir(key) || lookup_hir(key)) {
      return true;
    } else {
      insert(key, slow_get_page(key));
      return false;
    }
  }
};

}  // namespace caches

#pragma once

#include <cstddef>
#include <list>
#include <unordered_map>
#include <utility>

namespace native::ui {

template<typename K, typename V>
class LRUCache {
public:
  explicit LRUCache(size_t max_bytes) : max_bytes_(max_bytes) {}

  V* Get(const K& key) {
    auto it = map_.find(key);
    if (it == map_.end()) return nullptr;
    // Move to front (most recently used)
    list_.splice(list_.begin(), list_, it->second);
    return &it->second->value;
  }

  void Put(const K& key, V value) {
    auto it = map_.find(key);
    size_t entry_bytes = sizeof(key) + sizeof(value) + sizeof(Entry);

    if (it != map_.end()) {
      // Update existing — adjust total
      total_bytes_ -= it->second->bytes;
      it->second->value = std::move(value);
      it->second->bytes = entry_bytes;
      total_bytes_ += entry_bytes;
      list_.splice(list_.begin(), list_, it->second);
    } else {
      // Insert new
      list_.emplace_front(key, std::move(value), entry_bytes);
      map_[key] = list_.begin();
      total_bytes_ += entry_bytes;
    }

    // Evict until under limit
    while (total_bytes_ > max_bytes_ && !list_.empty()) {
      auto& last = list_.back();
      total_bytes_ -= last.bytes;
      map_.erase(last.key);
      list_.pop_back();
    }
  }

  void Clear() {
    list_.clear();
    map_.clear();
    total_bytes_ = 0;
  }

  size_t total_bytes() const { return total_bytes_; }

private:
  struct Entry {
    K key;
    V value;
    size_t bytes;
    Entry(K k, V v, size_t b)
        : key(std::move(k)), value(std::move(v)), bytes(b) {}
  };

  std::list<Entry> list_;
  std::unordered_map<K, typename std::list<Entry>::iterator> map_;
  size_t max_bytes_;
  size_t total_bytes_ = 0;
};

}  // namespace native::ui

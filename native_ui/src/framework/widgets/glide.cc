#include "glide.h"

#include <atomic>
#include <cstdio>
#include <future>
#include <mutex>
#include <unordered_map>

#include "lru_cache.h"

namespace native::ui {

// ── Global singleton ──

static Glide* g_glide = nullptr;

Glide* Glide::Default() { return g_glide; }
void Glide::SetDefault(Glide* glide) { g_glide = glide; }

// ── DefaultGlide ──

class DefaultGlide : public Glide {
public:
  DefaultGlide(size_t max_cache_bytes = 50 * 1024 * 1024,
               int thread_pool_size = 2)
      : cache_(max_cache_bytes) {
    (void)thread_pool_size;
  }

  uint64_t Load(const std::string& path,
                LoadCallback cb,
                const LoadOptions& opts) override {
    // Check cache first
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto* cached = cache_.Get(path);
      if (cached && *cached) {
        if (cb) cb(path, *cached, LoadState::kLoaded);
        return 0;
      }
    }

    // Submit async decode
    uint64_t id = ++next_id_;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto& req = requests_[id];
      req.path = path;
      req.cancelled = false;
      req.future = std::async(std::launch::async, [this, id, path, opts, cb]() {
        DoDecode(id, path, opts, cb);
      });
    }
    return id;
  }

  void Cancel(uint64_t id) override {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = requests_.find(id);
    if (it != requests_.end()) {
      it->second.cancelled = true;
      requests_.erase(it);
    }
  }

  void ClearCache() override {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.Clear();
  }

private:
  struct Request {
    std::string path;
    std::future<void> future;
    std::atomic<bool> cancelled{false};
  };

  void DoDecode(uint64_t id, const std::string& path,
                const LoadOptions& opts, LoadCallback cb) {
    // Check cancellation before decode
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (requests_.find(id) == requests_.end()) return;
      if (requests_[id].cancelled) return;
    }

    // Decode
    auto img = native::ui::Image::FromFile(path.c_str());
    if (!img) {
      if (cb) cb(path, nullptr, LoadState::kError);
      return;
    }

    // Cache
    {
      std::lock_guard<std::mutex> lock(mutex_);
      cache_.Put(path, std::move(img));
    }

    // Check cancellation again (avoid delivering stale result)
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (requests_.find(id) == requests_.end()) return;
      if (requests_[id].cancelled) return;
    }

    // Deliver on main thread
    if (cb) {
      auto* cached = cache_.Get(path);
      if (cached) {
        cb(path, *cached, LoadState::kLoaded);
      }
    }
  }

  std::mutex mutex_;
  LRUCache<std::string, std::shared_ptr<Image>> cache_;
  std::unordered_map<uint64_t, Request> requests_;
  uint64_t next_id_ = 0;
};

}  // namespace native::ui

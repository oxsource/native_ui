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

struct DefaultGlide::Impl {
  explicit Impl(size_t max_cache_bytes) : cache_(max_cache_bytes) {}

  struct Request {
    std::string path;
    std::future<void> future;
    std::atomic<bool> cancelled{false};
  };

  void DoDecode(uint64_t id, const std::string& path,
                const LoadOptions& opts, LoadCallback cb) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (requests_.find(id) == requests_.end()) return;
      if (requests_[id].cancelled) return;
    }

    auto img = native::ui::Image::FromFile(path.c_str());
    if (!img) {
      if (cb) cb(path, nullptr, LoadState::kError);
      return;
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      cache_.Put(path, std::move(img));
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (requests_.find(id) == requests_.end()) return;
      if (requests_[id].cancelled) return;
    }

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

DefaultGlide::DefaultGlide(size_t max_cache_bytes, int thread_pool_size)
    : impl_(std::make_unique<Impl>(max_cache_bytes)) {
  (void)thread_pool_size;
}

DefaultGlide::~DefaultGlide() = default;

uint64_t DefaultGlide::Load(const std::string& path,
                            LoadCallback cb,
                            const LoadOptions& opts) {
  {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    auto* cached = impl_->cache_.Get(path);
    if (cached && *cached) {
      if (cb) cb(path, *cached, LoadState::kLoaded);
      return 0;
    }
  }

  uint64_t id = ++impl_->next_id_;
  {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    auto& req = impl_->requests_[id];
    req.path = path;
    req.cancelled = false;
    req.future = std::async(std::launch::async, [this, id, path, opts, cb]() {
      impl_->DoDecode(id, path, opts, cb);
    });
  }
  return id;
}

void DefaultGlide::Cancel(uint64_t id) {
  std::lock_guard<std::mutex> lock(impl_->mutex_);
  auto it = impl_->requests_.find(id);
  if (it != impl_->requests_.end()) {
    it->second.cancelled = true;
    impl_->requests_.erase(it);
  }
}

void DefaultGlide::ClearCache() {
  std::lock_guard<std::mutex> lock(impl_->mutex_);
  impl_->cache_.Clear();
}

}  // namespace native::ui

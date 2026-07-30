#include "glide.h"

#include <atomic>
#include <cstdio>
#include <future>
#include <mutex>
#include <unordered_map>
#include <vector>

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
    std::vector<LoadCallback> callbacks;
  };

  void DoDecode(uint64_t id, const std::string& path) {
    auto img = native::ui::Image::FromFile(path.c_str());

    std::vector<LoadCallback> cbs;
    std::shared_ptr<Image> result;
    LoadState state = img ? LoadState::kLoaded : LoadState::kError;

    if (img) {
      std::lock_guard<std::mutex> lock(mutex_);
      cache_.Put(path, std::move(img));
      result = *cache_.Get(path);
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      pending_paths_.erase(path);
      auto it = requests_.find(id);
      if (it != requests_.end()) {
        cbs = std::move(it->second.callbacks);
      }
    }

    for (auto& cb : cbs) {
      if (cb) cb(path, result, state);
    }
  }

  std::mutex mutex_;
  LRUCache<std::string, std::shared_ptr<Image>> cache_;
  std::unordered_map<uint64_t, Request> requests_;
  std::unordered_map<std::string, uint64_t> pending_paths_;
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
  std::lock_guard<std::mutex> lock(impl_->mutex_);

  // Check cache first
  auto* cached = impl_->cache_.Get(path);
  if (cached && *cached) {
    if (cb) cb(path, *cached, LoadState::kLoaded);
    return 0;
  }

  // Check if already being loaded (deduplicate in-flight requests)
  auto pend = impl_->pending_paths_.find(path);
  if (pend != impl_->pending_paths_.end()) {
    uint64_t id = pend->second;
    auto it = impl_->requests_.find(id);
    if (it != impl_->requests_.end()) {
      if (cb) it->second.callbacks.push_back(std::move(cb));
      return id;
    }
  }

  // Start new async request
  uint64_t id = ++impl_->next_id_;
  auto& req = impl_->requests_[id];
  req.path = path;
  req.cancelled = false;
  if (cb) req.callbacks.push_back(std::move(cb));
  impl_->pending_paths_[path] = id;
  req.future = std::async(std::launch::async, [this, id, path]() {
    impl_->DoDecode(id, path);
  });
  return id;
}

void DefaultGlide::Cancel(uint64_t id) {
  std::lock_guard<std::mutex> lock(impl_->mutex_);
  auto it = impl_->requests_.find(id);
  if (it != impl_->requests_.end()) {
    impl_->pending_paths_.erase(it->second.path);
    it->second.cancelled = true;
    impl_->requests_.erase(it);
  }
}

void DefaultGlide::ClearCache() {
  std::lock_guard<std::mutex> lock(impl_->mutex_);
  impl_->cache_.Clear();
}

}  // namespace native::ui

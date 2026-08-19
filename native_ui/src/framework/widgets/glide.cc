#include "src/framework/widgets/glide.h"

#include <atomic>
#include <cstdio>
#include <future>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "src/framework/widgets/lru_cache.h"

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
    std::atomic<bool> done{false};
    std::vector<LoadCallback> callbacks;
    int target_width = 0;
    int target_height = 0;
  };

  struct PendingDelivery {
    std::string path;
    std::shared_ptr<Image> image;
    LoadState state;
    std::vector<LoadCallback> callbacks;
  };

  static std::unique_ptr<Image> ResizeImage(const Image& src,
                                             int tw, int th) {
    return src.Scale(tw, th);
  }

  void DoDecode(uint64_t id, const std::string& path) {
    auto img = Image::FromFile(path.c_str());

    PendingDelivery delivery;
    delivery.path = path;
    delivery.state = img ? LoadState::kLoaded : LoadState::kError;
    if (img) {
      int tw = 0, th = 0;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.Put(path, std::move(img));
        auto it = requests_.find(id);
        if (it != requests_.end()) {
          tw = it->second.target_width;
          th = it->second.target_height;
        }
      }
      auto* cached = cache_.Get(path);
      if (cached && *cached && (tw > 0 || th > 0)) {
        auto resized = ResizeImage(**cached, tw, th);
        delivery.image = resized ? std::move(resized) : *cached;
      } else if (cached) {
        delivery.image = *cached;
      }
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      pending_paths_.erase(path);
      auto it = requests_.find(id);
      if (it != requests_.end()) {
        delivery.callbacks = std::move(it->second.callbacks);
        it->second.done = true;
      }
    }

    if (!delivery.callbacks.empty()) {
      std::lock_guard<std::mutex> lock(mutex_);
      pending_deliveries_.push_back(std::move(delivery));
    }
  }

  void SweepStale() {
    for (auto it = requests_.begin(); it != requests_.end(); ) {
      if (it->second.done) {
        it = requests_.erase(it);
      } else {
        ++it;
      }
    }
  }

  std::mutex mutex_;
  LRUCache<std::string, std::shared_ptr<Image>> cache_;
  std::unordered_map<uint64_t, Request> requests_;
  std::unordered_map<std::string, uint64_t> pending_paths_;
  std::vector<PendingDelivery> pending_deliveries_;
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
  impl_->SweepStale();

  // Check cache first (use original-size cache entry, resize if needed)
  auto* cached = impl_->cache_.Get(path);
  if (cached && *cached) {
    std::shared_ptr<Image> result = *cached;
    if (opts.target_width > 0 || opts.target_height > 0) {
      auto resized = Impl::ResizeImage(**cached, opts.target_width, opts.target_height);
      if (resized) result = std::move(resized);
    }
    if (cb) cb(path, result, LoadState::kLoaded);
    return 0;
  }

  // Deduplicate in-flight requests
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
  req.target_width = opts.target_width;
  req.target_height = opts.target_height;
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

void DefaultGlide::DrainPendingCallbacks() {
  std::vector<Impl::PendingDelivery> deliveries;
  {
    std::lock_guard<std::mutex> lock(impl_->mutex_);
    deliveries = std::move(impl_->pending_deliveries_);
    impl_->pending_deliveries_.clear();
  }
  for (auto& d : deliveries) {
    for (auto& cb : d.callbacks) {
      if (cb) cb(d.path, d.image, d.state);
    }
  }
}

}  // namespace native::ui

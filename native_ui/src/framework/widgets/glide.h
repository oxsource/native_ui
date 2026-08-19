#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

#include "src/framework/render/image.h"

namespace native::ui {

enum class LoadState { kLoading, kLoaded, kError };

struct LoadOptions {
  int target_width = 0;
  int target_height = 0;
};

using LoadCallback = std::function<void(const std::string& path,
                                        std::shared_ptr<Image> image,
                                        LoadState state)>;

class Glide {
public:
  static Glide* Default();
  static void SetDefault(Glide* glide);

  virtual ~Glide() = default;

  virtual uint64_t Load(const std::string& file_path,
                        LoadCallback callback,
                        const LoadOptions& options = {}) = 0;
  virtual void Cancel(uint64_t request_id) = 0;
  virtual void ClearCache() = 0;
  virtual void DrainPendingCallbacks() = 0;
};

// Default LRU-backed implementation used by framework examples.
class DefaultGlide : public Glide {
public:
  explicit DefaultGlide(size_t max_cache_bytes = 50 * 1024 * 1024,
                        int thread_pool_size = 2);
  ~DefaultGlide() override;

  uint64_t Load(const std::string& file_path,
                LoadCallback callback,
                const LoadOptions& options = {}) override;
  void Cancel(uint64_t request_id) override;
  void ClearCache() override;
  void DrainPendingCallbacks() override;

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace native::ui

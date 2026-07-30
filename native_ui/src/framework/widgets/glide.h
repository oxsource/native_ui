#pragma once

#include <functional>
#include <memory>
#include <string>

#include "image.h"

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
};

}  // namespace native::ui

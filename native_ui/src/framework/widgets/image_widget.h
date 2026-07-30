#pragma once

#include <memory>
#include <string>

#include "glide.h"
#include "style.h"
#include "widget.h"

namespace native::ui {

struct ImagePath {
  std::string value;
};

struct ImageURI {
  std::string value;
};

class ImageWidget : public Widget {
public:
  template <typename... Args>
  explicit ImageWidget(Args&&... args) {
    (ProcessArg(std::forward<Args>(args)), ...);
  }

  using Widget::ProcessArg;

  ~ImageWidget() override;
  void OnUnmount() override;
  void Draw(Canvas& canvas) override;

private:
  // Non-style tags
  void ProcessArg(ImagePath tag);
  void ProcessArg(ImageURI tag)  { uri_ = tag.value; Load(); }
  void ProcessArg(Id tag)        { SetId(std::move(tag.value)); }

  // Style-delegating tags
  void ProcessArg(ScaleMode v)   { style_.setScaleType(v); }
  void ProcessArg(Gravity v)     { style_.setScaleGravity(v); }

  void Load();
  void Cancel();

  std::string uri_;
  std::shared_ptr<Image> loaded_image_;
  LoadState state_ = LoadState::kLoading;
  uint64_t request_id_ = 0;
  std::string load_key_;
};

}  // namespace native::ui

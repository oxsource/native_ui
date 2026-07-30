#pragma once

#include <memory>
#include <string>

#include "image.h"
#include "widget.h"

namespace native::ui {

struct ImagePath {
  std::string value;
};

class ImageWidget : public Widget {
public:
  template <typename... Args>
  explicit ImageWidget(Args&&... args);

  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(ImagePath tag);
  void ProcessArg(Id tag);

  std::string path_;
  std::unique_ptr<native::ui::Image> image_;
};

}  // namespace native::ui

#pragma once

#include <memory>

#include "hardware_buffer.h"
#include "image.h"
#include "widget.h"

namespace native::ui {

class ExternalImage : public Widget {
public:
  template <typename... Args>
  explicit ExternalImage(Args&&... args);

  void SetBuffer(HardwareBuffer buffer);
  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(HardwareBuffer tag);
  void ProcessArg(Id tag);

  HardwareBuffer buffer_;
  std::unique_ptr<native::ui::Image> image_;
  Property<HardwareBuffer>* watched_prop_ = nullptr;
};

}  // namespace native::ui

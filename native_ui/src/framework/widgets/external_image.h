#pragma once

#include <memory>

#include "hardware_buffer.h"
#include "canvas.h"
#include "widget.h"

namespace native::ui {

class ExternalImage : public Widget {
public:
  template <typename... Args>
  explicit ExternalImage(Args&&... args) {
    (ProcessArg(std::forward<Args>(args)), ...);
  }

  ~ExternalImage() override;

  void SetBuffer(HardwareBuffer buffer);
  void Watch(Property<HardwareBuffer>& prop);
  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(HardwareBuffer tag);
  void ProcessArg(Id tag);

  // Rebuilds image_ only when the bound handle changed (or no image exists yet).
  void UpdateBuffer(HardwareBuffer buffer);

  HardwareBuffer buffer_;
  std::unique_ptr<native::ui::Image> image_;
  Property<HardwareBuffer>* watched_prop_ = nullptr;
};

}  // namespace native::ui

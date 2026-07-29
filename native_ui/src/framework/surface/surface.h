#pragma once

#include <memory>

#include "hardware_buffer.h"

namespace native::ui {

class SurfaceImpl;

class Surface {
public:
  static std::unique_ptr<Surface> Create(int width, int height);
  static std::unique_ptr<Surface> CreateFromBuffer(HardwareBuffer buffer);
  ~Surface();

  void Flush();
  int width() const;
  int height() const;

private:
  Surface(SurfaceImpl* impl);
  SurfaceImpl* impl_ = nullptr;
};

}  // namespace native::ui

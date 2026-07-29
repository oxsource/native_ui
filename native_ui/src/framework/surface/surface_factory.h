#pragma once

#include <memory>

#include "hardware_buffer.h"

namespace native::ui {

class Surface;

class SurfaceFactory {
public:
  // Creates a platform-specific SkSurface from a hardware buffer.
  // Returns nullptr if the platform is unsupported or creation fails.
  static std::unique_ptr<Surface> CreateFromHardwareBuffer(HardwareBuffer buffer);
};

}  // namespace native::ui

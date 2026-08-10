#pragma once

#include <memory>

#include "hardware_buffer.h"
#include "surface.h"

namespace native::ui {

class SurfaceFactory {
public:
  // Creates a platform-specific SkSurface from a hardware buffer.
  // Returns nullptr if the platform is unsupported or creation fails.
  // `backend` selects CPU (raster) vs GPU (GLES/EGL); kGPU requires a non-null
  // RenderContext (falls back to CPU otherwise).
  static std::unique_ptr<Surface> CreateFromHardwareBuffer(
      HardwareBuffer buffer, RenderBackend backend = RenderBackend::kCPU,
      RenderContext* ctx = nullptr);
};

}  // namespace native::ui

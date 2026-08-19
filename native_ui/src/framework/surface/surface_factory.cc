#include "src/framework/surface/surface_factory.h"

#include "src/framework/surface/surface.h"

namespace native::ui {

std::unique_ptr<Surface> SurfaceFactory::CreateFromHardwareBuffer(
    HardwareBuffer buffer, RenderBackend backend, RenderContext* ctx) {
  if (!buffer.IsValid()) return nullptr;
  return Surface::CreateFromBuffer(buffer, backend, ctx);
}

}  // namespace native::ui

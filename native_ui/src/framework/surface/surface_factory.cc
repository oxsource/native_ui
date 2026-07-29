#include "surface_factory.h"

#include "surface.h"

namespace native::ui {

std::unique_ptr<Surface> SurfaceFactory::CreateFromHardwareBuffer(
    HardwareBuffer buffer) {
  if (!buffer.IsValid()) return nullptr;
  return Surface::CreateFromBuffer(buffer);
}

}  // namespace native::ui

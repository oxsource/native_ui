#include "surface.h"

#include "SkCanvas.h"
#include "SkSurface.h"

namespace native::ui {

class SurfaceImpl {
public:
  sk_sp<SkSurface> sk_surface;
};

Surface::Surface(SurfaceImpl* impl) : impl_(impl) {}

Surface::~Surface() { delete impl_; }

std::unique_ptr<Surface> Surface::Create(int width, int height) {
  auto* impl = new SurfaceImpl();
  auto info = SkImageInfo::MakeN32Premul(width, height);
  impl->sk_surface = SkSurfaces::Raster(info);
  if (!impl->sk_surface) {
    delete impl;
    return nullptr;
  }
  return std::unique_ptr<Surface>(new Surface(impl));
}

std::unique_ptr<Surface> Surface::CreateFromBuffer(HardwareBuffer buffer) {
  // MVP: fallback to raster. Platform-specific GPU surface creation
  // (IOSurface, DMA-BUF) will be implemented when hardware buffer
  // rendering is needed.
  if (!buffer.IsValid()) return nullptr;
  return Create(1024, 768);
}

SkCanvas* Surface::sk_canvas() const {
  return impl_->sk_surface->getCanvas();
}

void Surface::Flush() {
  // Raster surfaces do not require flushing.
  // GPU-backed surfaces (IOSurface, DMA-BUF) will call
  // the appropriate SkSurface flush method in the platform-specific path.
}

int Surface::width() const {
  return impl_ ? impl_->sk_surface->width() : 0;
}

int Surface::height() const {
  return impl_ ? impl_->sk_surface->height() : 0;
}

}  // namespace native::ui

#pragma once

#include <memory>

#include "SkRefCnt.h"
#include "hardware_buffer.h"

class SkCanvas;
class SkSurface;

namespace native::ui {

class RenderContext;
class SurfaceImpl;

// Rendering backend selection for buffer-backed surfaces/images. kCPU is the
// default (raster); kGPU is Android-only GLES/EGL and requires a non-null
// RenderContext (see contracts/render-backend.md).
enum class RenderBackend { kCPU, kGPU };

class Surface {
public:
  static std::unique_ptr<Surface> Create(int width, int height);
  static std::unique_ptr<Surface> CreateFromBuffer(HardwareBuffer buffer,
                                                   RenderBackend backend = RenderBackend::kCPU,
                                                   RenderContext* ctx = nullptr);

  // Internal: wrap an existing SkSurface (encoder-surface demo path).
  static std::unique_ptr<Surface> CreateFromSkSurface(sk_sp<SkSurface> sk_surface);

  ~Surface();

  void Flush();
  int width() const;
  int height() const;

  // Internal: accessed by Canvas and examples
  SkCanvas* sk_canvas() const;
  SkSurface* sk_surface() const;

private:
  friend class Canvas;
  Surface(SurfaceImpl* impl);
  SurfaceImpl* impl_ = nullptr;
};

}  // namespace native::ui

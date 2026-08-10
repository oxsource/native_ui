#pragma once

#include <memory>

#include "color.h"
#include "hardware_buffer.h"

namespace native::ui {

class RenderContext;
class SurfaceImpl;

// Rendering backend selection for buffer-backed surfaces/images. kCPU is the
// default (raster); kGPU is Android-only GLES/EGL and requires a non-null
// RenderContext (see contracts/render-backend.md).
enum class RenderBackend { kCPU, kGPU };

// A rendering target / backing store. Skia is fully encapsulated: no Skia types
// appear in this public interface (Skia isolation rules).
class Surface {
public:
  static std::unique_ptr<Surface> Create(int width, int height,
                                         ColorSpace color_space = ColorSpace::kSRGB);

  // Android: create a Surface whose canvas is hosted on the MediaCodec encoder
  // input surface (GLES/EGL render target, FBO 0). Dimensions/color space come from
  // the RenderContext (ctx->width/height/color_space). Returns nullptr on failure/host.
  static std::unique_ptr<Surface> Create(RenderContext* ctx);

  static std::unique_ptr<Surface> CreateFromBuffer(HardwareBuffer buffer,
                                                   RenderBackend backend = RenderBackend::kCPU,
                                                   RenderContext* ctx = nullptr);

  ~Surface();

  void Flush();
  int width() const;
  int height() const;

  // Dumps the surface's current pixels to a PNG file (sRGB, opaque).
  bool Dump(const char* path) const;

private:
  friend class Canvas;
  Surface(SurfaceImpl* impl);

  // Private (friend-only) renderer handle — opaque, not part of the public API.
  void* Handle() const;

  SurfaceImpl* impl_ = nullptr;
};

}  // namespace native::ui

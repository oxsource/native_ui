#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "src/framework/core/color.h"
#include "src/framework/surface/hardware_buffer.h"

namespace native::ui {

struct RenderContext;
class SurfaceImpl;

// Rendering backend selection for buffer-backed surfaces/images. kCPU is the
// default (raster); kGPU is Android-only GLES/EGL and requires a non-null
// RenderContext (see contracts/render-backend.md).
enum class RenderBackend { kCPU, kGPU };

// Pixel format for Surface::CreateFromPixels (wrapping an existing buffer).
// kRGBA is the native decode/readback format used across native::ui.
enum class PixelFormat { kRGBA, kBGRA };

// A self-describing, caller-owned pixel buffer: the byte data plus the
// dimensions/format/stridethat describe it. Produced by Surface::Allocate
// (which decides how much memory is needed) and consumed by
// Surface::CreateFromPixels(const PixelBuffer&). The caller owns `data`.
struct PixelBuffer {
  std::vector<uint8_t> data;                       // tightly-packed pixels
  int width = 0;
  int height = 0;
  PixelFormat format = PixelFormat::kRGBA;
  size_t row_bytes = 0;                            // 0 = width * 4

  bool empty() const { return data.empty() || width <= 0 || height <= 0; }
};

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

  // Allocates a self-describing PixelBuffer for a Surface of the given
  // dimensions/format. Surface decides how much memory is needed (width*height*
  // 4 for kRGBA/kBGRA) and fills in width/height/format. The caller OWNS the
  // returned buffer (lifetime is external) and hands it to
  // CreateFromPixels(const PixelBuffer&), which wraps it zero-copy. Returns an
  // empty PixelBuffer on invalid arguments or unsupported format.
  static PixelBuffer Allocate(int width, int height,
                                  PixelFormat format = PixelFormat::kRGBA);

  // Wraps a caller-owned PixelBuffer (from Allocate) as a writable raster
  // Surface. Zero-copy: the surface does NOT own pb.data; the caller must keep
  // it alive and mutable for the surface's lifetime. This mirrors the Android
  // encoder input-surface model (Create(ctx)) and lets consumers hand decoded
  // image bytes (e.g. a decoded background/car) directly to a canvas without an
  // encode/decode round-trip. `pb` is taken by non-const reference because the
  // wrapped surface is writable. Returns nullptr on invalid args/unsupported
  // format.
  static std::unique_ptr<Surface> CreateFromPixels(PixelBuffer& pb);

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

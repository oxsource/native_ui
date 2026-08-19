#pragma once

#include <memory>

#include "src/framework/surface/hardware_buffer.h"
#include "src/framework/surface/surface.h"  // RenderBackend, RenderContext

namespace native::ui {

class ImageImpl;

// A drawable image source (decoded file/encoded data or external buffer). Skia is
// fully encapsulated: no Skia types appear in this public interface.
class Image {
public:
  static std::unique_ptr<Image> FromEncoded(const void* data, size_t size);
  static std::unique_ptr<Image> FromFile(const char* path);
  static std::unique_ptr<Image> FromBuffer(HardwareBuffer buffer,
                                           RenderBackend backend = RenderBackend::kCPU,
                                           RenderContext* ctx = nullptr);

  ~Image();

  int width() const;
  int height() const;

  // Copies the raw RGBA pixels (image's alpha type) into `dst`, `row_bytes` per row.
  bool CopyPixels(int width, int height, size_t row_bytes, void* dst) const;

  // Returns a new Image scaled to fit `tw`x`th` (aspect preserved, centered on a
  // transparent background), or nullptr if no resize is needed.
  std::unique_ptr<Image> Scale(int tw, int th) const;

private:
  friend class Canvas;
  Image();

  // Private (friend-only) renderer handle — opaque, not part of the public API.
  void* Handle() const;

  // Internal SVG load (image.cc).
  static std::unique_ptr<Image> LoadSvg(const char* path);

  ImageImpl* impl_ = nullptr;
};

}  // namespace native::ui

#pragma once

#include <memory>

#include "hardware_buffer.h"
#include "surface.h"  // RenderBackend, RenderContext

#include "SkImage.h"

namespace native::ui {

class ImageImpl;

class Image {
public:
  static std::unique_ptr<Image> FromEncoded(const void* data, size_t size);
  static std::unique_ptr<Image> FromFile(const char* path);
  static std::unique_ptr<Image> FromBuffer(HardwareBuffer buffer,
                                           RenderBackend backend = RenderBackend::kCPU,
                                           RenderContext* ctx = nullptr);

  // Internal: wrap an existing SkImage (used by nanosvg SVG path)
  static std::unique_ptr<Image> FromSkImage(sk_sp<SkImage> sk_image);

  ~Image();

  int width() const;
  int height() const;

  SkImage* sk_image() const;

private:
  friend class Canvas;

  Image();
  ImageImpl* impl_ = nullptr;
};

}  // namespace native::ui

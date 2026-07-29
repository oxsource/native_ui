#pragma once

#include <memory>

#include "hardware_buffer.h"

class SkImage;

namespace native::ui {

class ImageImpl;

class Image {
public:
  static std::unique_ptr<Image> FromEncoded(const void* data, size_t size);
  static std::unique_ptr<Image> FromFile(const char* path);
  static std::unique_ptr<Image> FromBuffer(HardwareBuffer buffer);

  ~Image();

  int width() const;
  int height() const;

private:
  friend class Canvas;
  SkImage* sk_image() const;

  Image();
  ImageImpl* impl_ = nullptr;
};

}  // namespace native::ui

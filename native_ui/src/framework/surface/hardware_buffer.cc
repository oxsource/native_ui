#include "hardware_buffer.h"

#include <cstdint>

#if defined(__ANDROID__)
#include <android/hardware_buffer.h>
#endif

namespace native::ui {

namespace {

// Bytes per pixel for the formats the framework supports. Returns 0 for unknown.
#if defined(__ANDROID__)
int BytesPerPixel(int format) {
  switch (format) {
    case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
      return 4;
    default:
      return 0;
  }
}
#endif

}  // namespace

HardwareBuffer::HardwareBuffer(Kind kind, void* handle, void* pixels, size_t row_bytes,
                               int width, int height, int format)
    : kind_(kind),
      handle_(handle),
      pixels_(pixels),
      row_bytes_(row_bytes),
      width_(width),
      height_(height),
      format_(format) {
  geometry_described_ = (kind != Kind::kAHardwareBuffer);  // Memory stores geometry directly
  // AHardwareBuffer dims are lazily described (0 at wrap time), so validity for that
  // kind only requires a non-null handle; Memory requires explicit positive dims.
  if (kind == Kind::kAHardwareBuffer) {
    valid_ = handle != nullptr;
  } else {
    valid_ = kind != Kind::kInvalid && width > 0 && height > 0 && handle != nullptr;
  }
}

#if __APPLE__
HardwareBuffer HardwareBuffer::FromIOSurface(void* iosurface) {
  // TODO(android-only): IOSurface support is reserved for a future feature.
  (void)iosurface;
  return {};
}
#elif __ANDROID__
HardwareBuffer HardwareBuffer::FromAHardwareBuffer(void* buffer) {
  if (!buffer) return {};
  // Non-owning: the producer retains lifetime (FR-011). Geometry is filled lazily
  // by EnsureDescribe() from AHardwareBuffer_describe on first access.
  return HardwareBuffer(Kind::kAHardwareBuffer, buffer, nullptr, 0, 0, 0, 0);
}
#elif __linux__
HardwareBuffer HardwareBuffer::FromDmaBuf(int fd) {
  // TODO(android-only): DMA-BUF support is reserved for a future feature.
  (void)fd;
  return {};
}
#endif

HardwareBuffer HardwareBuffer::FromMemory(void* pixels, size_t row_bytes, int width,
                                          int height) {
  if (!pixels || width <= 0 || height <= 0 || row_bytes == 0) return {};
  return HardwareBuffer(Kind::kMemory, pixels, pixels, row_bytes, width, height, 0);
}

void HardwareBuffer::EnsureDescribe() const {
  if (geometry_described_ || kind_ != Kind::kAHardwareBuffer) return;
  geometry_described_ = true;
#if defined(__ANDROID__)
  AHardwareBuffer_Desc desc{};
  AHardwareBuffer_describe(static_cast<AHardwareBuffer*>(handle_), &desc);
  width_ = static_cast<int>(desc.width);
  height_ = static_cast<int>(desc.height);
  format_ = static_cast<int>(desc.format);
  row_bytes_ = static_cast<size_t>(desc.stride) * BytesPerPixel(static_cast<int>(desc.format));
#endif
}

int HardwareBuffer::width() const {
  EnsureDescribe();
  return width_;
}

int HardwareBuffer::height() const {
  EnsureDescribe();
  return height_;
}

size_t HardwareBuffer::row_bytes() const {
  EnsureDescribe();
  return row_bytes_;
}

int HardwareBuffer::format() const {
  EnsureDescribe();
  return format_;
}

}  // namespace native::ui

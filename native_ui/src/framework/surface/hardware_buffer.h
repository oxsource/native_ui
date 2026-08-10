#pragma once

#include <cstddef>
#include <cstdint>

namespace native::ui {

// Cross-platform wrapper around a platform image buffer (Android AHardwareBuffer,
// host CPU memory, ...). Non-owning: the producer retains the buffer (FR-011);
// the framework copies whatever it renders.
class HardwareBuffer {
public:
  enum class Kind {
    kMemory,          // host CPU pixels (HardwareBuffer::FromMemory)
    kAHardwareBuffer, // Android AHardwareBuffer
    kInvalid,
  };

#if __APPLE__
  static HardwareBuffer FromIOSurface(void* iosurface);
#elif __ANDROID__
  static HardwareBuffer FromAHardwareBuffer(void* buffer);
#elif __linux__
  static HardwareBuffer FromDmaBuf(int fd);
#endif

  // Data-only factory (all platforms): wraps host CPU pixel memory. No rendering;
  // kept for wrapper tests and reserved for future non-Android use.
  static HardwareBuffer FromMemory(void* pixels, size_t row_bytes, int width, int height);

  HardwareBuffer() = default;

  bool IsValid() const { return valid_; }
  Kind kind() const { return kind_; }

  int width() const;
  int height() const;
  size_t row_bytes() const;
  int format() const;  // 0 = unknown

  // Compares the underlying handle; used by ExternalImage to skip redundant
  // per-frame re-conversion (FR-003/FR-007).
  bool operator==(const HardwareBuffer& other) const { return handle_ == other.handle_; }

  // CPU pixel pointer (Memory kind only).
  const void* pixels() const { return pixels_; }

#if __APPLE__
  void* iosurface() const { return handle_; }
#elif __ANDROID__
  void* ahardwarebuffer() const { return handle_; }
#elif __linux__
  int dma_buf_fd() const { return static_cast<int>(reinterpret_cast<intptr_t>(handle_)); }
#endif

private:
  HardwareBuffer(Kind kind, void* handle, void* pixels, size_t row_bytes,
                 int width, int height, int format);

  // For AHardwareBuffer kind: fills geometry lazily from AHardwareBuffer_describe.
  void EnsureDescribe() const;

  Kind kind_ = Kind::kInvalid;
  void* handle_ = nullptr;   // AHardwareBuffer* (Android) or pixel pointer (Memory)
  void* pixels_ = nullptr;   // Memory kind only
  mutable size_t row_bytes_ = 0;
  mutable int width_ = 0;
  mutable int height_ = 0;
  mutable int format_ = 0;
  mutable bool geometry_described_ = false;
  bool valid_ = false;
};

}  // namespace native::ui

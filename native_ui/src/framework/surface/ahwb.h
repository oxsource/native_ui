#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

#include "SkRefCnt.h"

class SkImage;
class GrDirectContext;
struct AHardwareBuffer;  // defined by <android/hardware_buffer.h> on Android

namespace native::ui {

// Android AHardwareBuffer utility layer, modeled on falcon's AHwbPool (no OpenCV).
// All functions are Android-only; on host builds they are stubs returning negative
// status codes (-5 = non-Android platform), keeping host/CI builds green.
class AHwb {
public:
  // Wraps AHardwareBuffer_describe. Returns false on null buffer or failure.
  static bool Describe(AHardwareBuffer* buffer, uint32_t& w, uint32_t& h,
                       uint32_t& stride, int& format);

  // AHardwareBuffer_lock with fence = -1, rect = nullptr.
  static int Lock(AHardwareBuffer* buffer, uint64_t usage, void** data);

  // AHardwareBuffer_unlock with fence = nullptr.
  static int Unlock(AHardwareBuffer* buffer);

  // Lock -> invoke fn -> unlock. Unlock is guaranteed even if fn returns early (no exceptions).
  static int Pixels(AHardwareBuffer* buffer, uint64_t usage,
                    const std::function<void(void*)>& fn);

  // AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM; usage CPU_READ_OFTEN|CPU_WRITE_OFTEN|
  // GPU_SAMPLED_IMAGE|GPU_COLOR_OUTPUT; layers = 1. Returns null on failure.
  static AHardwareBuffer* AllocateRgba(uint32_t w, uint32_t h);

  // Lock CPU_WRITE_OFTEN; copy height rows of min(width*4, src_row_bytes, dst_row_bytes); unlock.
  static int WriteRgba(AHardwareBuffer* buffer, const uint8_t* src, size_t src_row_bytes);

  // AHardwareBuffer_release; null-safe.
  static void Release(AHardwareBuffer* buffer);

  // Describe -> lock CPU_READ_OFTEN -> build RGBA8888 SkImage (owned copy honoring stride) -> unlock.
  static sk_sp<SkImage> ToCpuImage(AHardwareBuffer* buffer, bool copy = true);

  // Zero-copy GPU texture import (GrAHardwareBufferUtils, GLES/EGL). Requires non-null context.
  static sk_sp<SkImage> ToGpuImage(AHardwareBuffer* buffer, GrDirectContext* gr);

  // Read via CPU lock -> encode PNG -> write (diagnostics, FR-010).
  static int DumpPng(AHardwareBuffer* buffer, const char* path);
};

}  // namespace native::ui

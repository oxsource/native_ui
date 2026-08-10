#include "ahwb.h"

#include <algorithm>
#include <cstring>

#if defined(__ANDROID__)
#include <android/hardware_buffer.h>

#include "SkBitmap.h"
#include "SkColorSpace.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkImageInfo.h"
#include "SkPixmap.h"
#include "include/android/GrAHardwareBufferUtils.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "stb_image_write.h"
#endif

namespace native::ui {

#if !defined(__ANDROID__)

// Host stubs: non-Android platform (-5), mirroring the falcon error idiom.

bool AHwb::Describe(AHardwareBuffer*, uint32_t&, uint32_t&, uint32_t&, int&) {
  // TODO(android-only): stub keeps host/CI builds green.
  return false;
}
int AHwb::Lock(AHardwareBuffer*, uint64_t, void**) { return -5; }
int AHwb::Unlock(AHardwareBuffer*) { return -5; }
int AHwb::Pixels(AHardwareBuffer*, uint64_t, const std::function<void(void*)>&) { return -5; }
AHardwareBuffer* AHwb::AllocateRgba(uint32_t, uint32_t) { return nullptr; }
int AHwb::WriteRgba(AHardwareBuffer*, const uint8_t*, size_t) { return -5; }
void AHwb::Release(AHardwareBuffer*) {}
sk_sp<SkImage> AHwb::ToCpuImage(AHardwareBuffer*, bool) { return nullptr; }
sk_sp<SkImage> AHwb::ToGpuImage(AHardwareBuffer*, GrDirectContext*) { return nullptr; }
int AHwb::DumpPng(AHardwareBuffer*, const char*) { return -5; }

#else  // defined(__ANDROID__)

namespace {

constexpr uint64_t kRgbaUsage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
                               AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN |
                               AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE |
                               AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT;

// Only R8G8B8A8_UNORM is supported (FR-006); other formats are rejected.
bool IsSupportedFormat(int format) {
  return format == AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
}

}  // namespace

bool AHwb::Describe(AHardwareBuffer* buffer, uint32_t& w, uint32_t& h, uint32_t& stride,
                    int& format) {
  if (!buffer) return false;
  AHardwareBuffer_Desc desc{};
  AHardwareBuffer_describe(buffer, &desc);
  w = desc.width;
  h = desc.height;
  stride = desc.stride;  // row stride in pixels
  format = static_cast<int>(desc.format);
  return true;
}

int AHwb::Lock(AHardwareBuffer* buffer, uint64_t usage, void** data) {
  if (!buffer || !data) return -1;
  // fence = -1 (wait for prior producers), rect = nullptr (entire buffer).
  return AHardwareBuffer_lock(buffer, usage, -1, nullptr, data);
}

int AHwb::Unlock(AHardwareBuffer* buffer) {
  if (!buffer) return -1;
  return AHardwareBuffer_unlock(buffer, nullptr);
}

int AHwb::Pixels(AHardwareBuffer* buffer, uint64_t usage,
                 const std::function<void(void*)>& fn) {
  if (!buffer || !fn) return -1;
  void* data = nullptr;
  int rc = Lock(buffer, usage, &data);
  if (rc != 0) return rc;
  fn(data);
  return Unlock(buffer);  // guaranteed even if fn returned early (no exceptions)
}

AHardwareBuffer* AHwb::AllocateRgba(uint32_t w, uint32_t h) {
  if (w == 0 || h == 0) return nullptr;
  AHardwareBuffer_Desc desc{};
  desc.width = w;
  desc.height = h;
  desc.layers = 1;
  desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
  desc.usage = kRgbaUsage;
  AHardwareBuffer* buffer = nullptr;
  if (AHardwareBuffer_allocate(&desc, &buffer) != 0) return nullptr;
  return buffer;
}

int AHwb::WriteRgba(AHardwareBuffer* buffer, const uint8_t* src, size_t src_row_bytes) {
  if (!buffer || !src) return -1;
  uint32_t w = 0, h = 0, stride = 0;
  int format = 0;
  if (!Describe(buffer, w, h, stride, format)) return -2;
  if (!IsSupportedFormat(format)) return -2;  // FR-006: defined error, no corrupt output
  const size_t dst_row_bytes = static_cast<size_t>(stride) * 4;
  void* data = nullptr;
  int rc = Lock(buffer, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, &data);
  if (rc != 0) return rc;
  const uint8_t* src_row = src;
  uint8_t* dst_row = static_cast<uint8_t*>(data);
  const size_t copy_bytes = std::min<size_t>({static_cast<size_t>(w) * 4, src_row_bytes,
                                              dst_row_bytes});
  for (uint32_t y = 0; y < h; ++y) {
    std::memcpy(dst_row + y * dst_row_bytes, src_row + y * src_row_bytes, copy_bytes);
  }
  return Unlock(buffer);
}

void AHwb::Release(AHardwareBuffer* buffer) {
  if (buffer) AHardwareBuffer_release(buffer);
}

sk_sp<SkImage> AHwb::ToCpuImage(AHardwareBuffer* buffer, bool copy) {
  if (!buffer) return nullptr;
  uint32_t w = 0, h = 0, stride = 0;
  int format = 0;
  if (!Describe(buffer, w, h, stride, format)) return nullptr;
  if (!IsSupportedFormat(format)) return nullptr;  // FR-006
  if (w == 0 || h == 0) return nullptr;            // FR-005: zero-area renders nothing
  const size_t dst_row_bytes = static_cast<size_t>(stride) * 4;
  void* data = nullptr;
  if (Lock(buffer, AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN, &data) != 0) return nullptr;

  sk_sp<SkData> pixels;
  if (copy) {
    pixels = SkData::MakeWithCopy(data, dst_row_bytes * h);  // owned copy (FR-011)
  } else {
    pixels = SkData::MakeWithoutCopy(data, dst_row_bytes * h);  // borrow; buffer must stay valid
  }
  auto info = SkImageInfo::Make(static_cast<int>(w), static_cast<int>(h), kRGBA_8888_SkColorType,
                                kPremul_SkAlphaType);
  sk_sp<SkImage> image = SkImages::RasterFromData(info, std::move(pixels), dst_row_bytes);
  Unlock(buffer);
  return image;
}

sk_sp<SkImage> AHwb::ToGpuImage(AHardwareBuffer* buffer, GrDirectContext* gr) {
  if (!buffer || !gr) return nullptr;
  uint32_t w = 0, h = 0, stride = 0;
  int format = 0;
  if (!Describe(buffer, w, h, stride, format)) return nullptr;
  if (!IsSupportedFormat(format)) return nullptr;  // FR-006
  if (w == 0 || h == 0) return nullptr;            // FR-005: zero-area renders nothing

  GrAHardwareBufferUtils::DeleteImageProc delete_proc = nullptr;
  GrAHardwareBufferUtils::UpdateImageProc update_proc = nullptr;
  GrAHardwareBufferUtils::TexImageCtx image_ctx = nullptr;
  GrBackendFormat backend_format = GrAHardwareBufferUtils::GetGLBackendFormat(
      gr, static_cast<uint32_t>(format), /*requireKnownFormat=*/true);
  GrBackendTexture tex = GrAHardwareBufferUtils::MakeGLBackendTexture(
      gr, buffer, static_cast<int>(w), static_cast<int>(h), &delete_proc, &update_proc,
      &image_ctx, /*isProtectedContent=*/false, backend_format, /*isRenderable=*/true);
  if (!tex.isValid() || !delete_proc) return nullptr;

  // Borrow: Skia calls delete_proc(image_ctx) when the image is released (render thread).
  return SkImages::BorrowTextureFrom(gr, tex, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
                                     kPremul_SkAlphaType, /*colorSpace=*/nullptr, delete_proc,
                                     image_ctx);
}

int AHwb::DumpPng(AHardwareBuffer* buffer, const char* path) {
  if (!buffer || !path) return -1;
  uint32_t w = 0, h = 0, stride = 0;
  int format = 0;
  if (!Describe(buffer, w, h, stride, format)) return -2;
  if (!IsSupportedFormat(format)) return -2;
  void* data = nullptr;
  int rc = Lock(buffer, AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN, &data);
  if (rc != 0) return -3;
  const int row_bytes = static_cast<int>(stride) * 4;
  const int ok = stbi_write_png(path, static_cast<int>(w), static_cast<int>(h), 4, data, row_bytes);
  Unlock(buffer);
  return ok == 0 ? -4 : 1;
}

#endif  // defined(__ANDROID__)

}  // namespace native::ui

#include "image.h"

#include <cstring>

#include "SkBitmap.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkImageInfo.h"
#include "SkPixmap.h"
#include "stb_image.h"
#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg.h"
#include "nanosvgrast.h"

#if defined(__ANDROID__)
#include <android/hardware_buffer.h>

#include "ahwb.h"
#include "render_context.h"
#endif

namespace native::ui {

namespace {

constexpr char kSvgExt[] = "svg";
constexpr char kSvgExtUpper[] = "SVG";
constexpr char kNanosvgUnits[] = "px";
// CSS standard default: 1px = 1/96 inch at 96dpi.
constexpr float kNanosvgDpi = 96.0f;
constexpr int kRgbaChannels = 4;

}  // namespace

class ImageImpl {
public:
  sk_sp<SkImage> sk_image;
};

Image::Image() : impl_(new ImageImpl()) {}

Image::~Image() { delete impl_; }

std::unique_ptr<Image> Image::FromEncoded(const void* data, size_t size) {
  auto sk_data = SkData::MakeWithCopy(data, size);
  auto sk_image = SkImages::DeferredFromEncodedData(sk_data);
  if (!sk_image) return nullptr;

  auto img = std::unique_ptr<Image>(new Image());
  img->impl_->sk_image = sk_image;
  return img;
}

static std::unique_ptr<Image> LoadSVG(const char* path) {
  NSVGimage* svg = nsvgParseFromFile(path, kNanosvgUnits, kNanosvgDpi);
  if (!svg) return nullptr;

  int w = static_cast<int>(svg->width);
  int h = static_cast<int>(svg->height);
  if (w <= 0 || h <= 0) { nsvgDelete(svg); return nullptr; }

  unsigned char* rgba = new unsigned char[w * h * kRgbaChannels];
  NSVGrasterizer* rast = nsvgCreateRasterizer();
  nsvgRasterize(rast, svg, 0, 0, 1.0f, rgba, w, h, w * kRgbaChannels);
  nsvgDeleteRasterizer(rast);
  nsvgDelete(svg);

  auto info = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
  auto bm = SkBitmap();
  bm.installPixels(info, rgba, w * kRgbaChannels,
      [](void* p, void*) { delete[] static_cast<unsigned char*>(p); }, nullptr);
  auto sk_image = bm.asImage();
  if (!sk_image) return nullptr;

  return Image::FromSkImage(sk_image);
}

std::unique_ptr<Image> Image::FromFile(const char* path) {
  if (!path) return nullptr;

  // Detect SVG by extension
  const char* dot = std::strrchr(path, '.');
  if (dot) {
    auto ext = dot + 1;
    if (std::strcmp(ext, kSvgExt) == 0 || std::strcmp(ext, kSvgExtUpper) == 0) {
      return LoadSVG(path);
    }
  }

  int w = 0, h = 0, channels = 0;
  unsigned char* rgba = stbi_load(path, &w, &h, &channels, kRgbaChannels);
  if (!rgba) return nullptr;

  auto info = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
  auto bm = SkBitmap();
  bm.installPixels(info, rgba, w * kRgbaChannels,
      [](void* p, void*) { stbi_image_free(p); }, nullptr);
  auto sk_image = bm.asImage();
  if (!sk_image) return nullptr;

  auto img = std::unique_ptr<Image>(new Image());
  img->impl_->sk_image = sk_image;
  return img;
}

std::unique_ptr<Image> Image::FromBuffer(HardwareBuffer buffer, RenderBackend backend,
                                         RenderContext* ctx) {
  if (!buffer.IsValid()) return nullptr;

#if defined(__ANDROID__)
  if (buffer.kind() != HardwareBuffer::Kind::kAHardwareBuffer) {
    // Memory kind has no Android render path (reserved; TODO(android-only)).
    return nullptr;
  }
  // FR-005: zero-area buffers render nothing.
  if (buffer.width() <= 0 || buffer.height() <= 0) return nullptr;
  // FR-006: defined error state for unsupported formats — never corrupt output.
  // AHardwareBuffer buffers always carry a real format from AHardwareBuffer_describe;
  // 0 means "unknown" and is rejected here for this kind.
  if (buffer.format() != AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM) return nullptr;

  AHardwareBuffer* ahwb = static_cast<AHardwareBuffer*>(buffer.ahardwarebuffer());
  if (!ahwb) return nullptr;

  RenderBackend effective = backend;
  if (effective == RenderBackend::kGPU && (!ctx || !ctx->gr)) {
    effective = RenderBackend::kCPU;  // fall back to CPU per contracts/render-backend.md
  }

  // CPU: owned copy (FR-011). GPU: zero-copy texture import (AHwb returns nullptr for
  // unsupported formats — FR-006).
  sk_sp<SkImage> sk_image = (effective == RenderBackend::kGPU)
                                ? AHwb::ToGpuImage(ahwb, ctx->gr)
                                : AHwb::ToCpuImage(ahwb, /*copy=*/true);
  if (!sk_image) return nullptr;

  auto img = std::unique_ptr<Image>(new Image());
  img->impl_->sk_image = std::move(sk_image);
  return img;
#else
  (void)backend;
  (void)ctx;
  // TODO(android-only): host builds stub the buffer-to-image conversion.
  return nullptr;
#endif
}

std::unique_ptr<Image> Image::FromSkImage(sk_sp<SkImage> sk_image) {
  if (!sk_image) return nullptr;
  auto img = std::unique_ptr<Image>(new Image());
  img->impl_->sk_image = std::move(sk_image);
  return img;
}

int Image::width() const {
  return impl_->sk_image ? impl_->sk_image->width() : 0;
}

int Image::height() const {
  return impl_->sk_image ? impl_->sk_image->height() : 0;
}

SkImage* Image::sk_image() const {
  return impl_->sk_image.get();
}

}  // namespace native::ui

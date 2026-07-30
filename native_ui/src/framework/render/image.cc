#include "image.h"

#include <cstring>

#include "SkBitmap.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkImageInfo.h"
#include "SkPixmap.h"
#include "nanosvg.h"
#include "nanosvgrast.h"

namespace native::ui {

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
  NSVGimage* svg = nsvgParseFromFile(path, "px", 96.0f);
  if (!svg) return nullptr;

  int w = static_cast<int>(svg->width);
  int h = static_cast<int>(svg->height);
  if (w <= 0 || h <= 0) { nsvgDelete(svg); return nullptr; }

  unsigned char* rgba = new unsigned char[w * h * 4];
  NSVGrasterizer* rast = nsvgCreateRasterizer();
  nsvgRasterize(rast, svg, 0, 0, 1.0f, rgba, w, h, w * 4);
  nsvgDeleteRasterizer(rast);
  nsvgDelete(svg);

  auto info = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
  auto bm = SkBitmap();
  bm.installPixels(info, rgba, w * 4,
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
    if (std::strcmp(ext, "svg") == 0 || std::strcmp(ext, "SVG") == 0) {
      return LoadSVG(path);
    }
  }

  // Raster formats (PNG, JPEG, WebP)
  auto sk_data = SkData::MakeFromFileName(path);
  if (!sk_data) return nullptr;
  auto sk_image = SkImages::DeferredFromEncodedData(sk_data);
  if (!sk_image) return nullptr;

  auto img = std::unique_ptr<Image>(new Image());
  img->impl_->sk_image = sk_image;
  return img;
}

std::unique_ptr<Image> Image::FromBuffer(HardwareBuffer buffer) {
  (void)buffer;
  return nullptr;
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

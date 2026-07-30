#include "image.h"

#include "SkData.h"
#include "SkImage.h"

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

std::unique_ptr<Image> Image::FromFile(const char* path) {
  auto sk_data = SkData::MakeFromFileName(path);
  if (!sk_data) return nullptr;

  auto sk_image = SkImages::DeferredFromEncodedData(sk_data);
  if (!sk_image) return nullptr;

  auto img = std::unique_ptr<Image>(new Image());
  img->impl_->sk_image = sk_image;
  return img;
}

std::unique_ptr<Image> Image::FromBuffer(HardwareBuffer buffer) {
  auto img = std::unique_ptr<Image>(new Image());
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

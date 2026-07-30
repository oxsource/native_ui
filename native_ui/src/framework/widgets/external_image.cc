#include "external_image.h"

#include "canvas.h"
#include "image.h"

namespace native::ui {

ExternalImage::~ExternalImage() = default;

void ExternalImage::ProcessArg(HardwareBuffer tag) {
  buffer_ = tag;
  if (buffer_.IsValid()) {
    image_ = native::ui::Image::FromBuffer(buffer_);
  }
}

void ExternalImage::ProcessArg(Id tag) { SetId(std::move(tag.value)); }

void ExternalImage::SetBuffer(HardwareBuffer buffer) {
  buffer_ = buffer;
  if (buffer_.IsValid()) {
    image_ = native::ui::Image::FromBuffer(buffer_);
  } else {
    image_.reset();
  }
  RequestRedraw();
}

void ExternalImage::Watch(Property<HardwareBuffer>& prop) {
  watched_prop_ = &prop;
  Widget::Watch(prop);
}

void ExternalImage::Draw(Canvas& canvas) {
  if (watched_prop_ && watched_prop_->value().IsValid()) {
    image_ = native::ui::Image::FromBuffer(watched_prop_->value());
  }
  if (!image_) return;
  Rect bb = bounds();
  if (bb.width <= 0 || bb.height <= 0) return;
  canvas.DrawImage(*image_, bb);
}

}  // namespace native::ui

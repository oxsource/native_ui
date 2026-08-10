#include "external_image.h"

#include "canvas.h"
#include "image.h"

namespace native::ui {

ExternalImage::~ExternalImage() = default;

void ExternalImage::ProcessArg(HardwareBuffer tag) {
  UpdateBuffer(tag);
}

void ExternalImage::ProcessArg(Id tag) { SetId(std::move(tag.value)); }

void ExternalImage::SetBuffer(HardwareBuffer buffer) {
  UpdateBuffer(buffer);
  RequestRedraw();
}

void ExternalImage::Watch(Property<HardwareBuffer>& prop) {
  watched_prop_ = &prop;
  Widget::Watch(prop);
}

// Rebuild image_ only when the underlying handle actually changed (or there is no
// image yet), so a static frame is not re-copied on every draw and 30fps updates
// are cheap (FR-003/FR-007). Uses HardwareBuffer::operator== (handle compare).
void ExternalImage::UpdateBuffer(HardwareBuffer buffer) {
  if (buffer == buffer_ && image_) return;  // redundant rebuild guard
  buffer_ = buffer;
  if (buffer_.IsValid()) {
    // Deterministic default backend: CPU (raster); GPU is opt-in via Surface/RenderContext.
    image_ = native::ui::Image::FromBuffer(buffer_, RenderBackend::kCPU);
  } else {
    image_.reset();
  }
}

void ExternalImage::Draw(Canvas& canvas) {
  if (watched_prop_) {
    HardwareBuffer value = watched_prop_->value();
    if (value != buffer_ || !image_) {
      UpdateBuffer(value);
    }
  }
  if (!image_) return;
  Rect bb = bounds();
  if (bb.width <= 0 || bb.height <= 0) return;
  canvas.DrawImage(*image_, bb);
}

}  // namespace native::ui

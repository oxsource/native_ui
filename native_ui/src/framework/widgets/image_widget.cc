#include "image_widget.h"
#include "canvas.h"

namespace native::ui {

ImageWidget::~ImageWidget() = default;

void ImageWidget::ProcessArg(ImagePath tag) {
  path_ = std::move(tag.value);
  if (!path_.empty()) {
    image_ = native::ui::Image::FromFile(path_.c_str());
  }
}

void ImageWidget::ProcessArg(Id tag) { SetId(std::move(tag.value)); }

void ImageWidget::Draw(Canvas& canvas) {
  if (!image_) return;
  Rect bb = bounds();
  if (bb.width <= 0 || bb.height <= 0) return;
  canvas.DrawImage(*image_, bb);
}

}  // namespace native::ui

#include "image_widget.h"
#include "canvas.h"
#include "paint.h"

#include <cmath>

namespace native::ui {

ImageWidget::~ImageWidget() { Cancel(); }

void ImageWidget::OnMount() {
  if (!uri_.empty() && state_ == LoadState::kLoading) {
    Load();
  }
}

void ImageWidget::OnUnmount() {
  Cancel();
  Widget::OnUnmount();
}

void ImageWidget::Load() {
  Cancel();  // cancel previous request

  auto* glide = Glide::Default();
  if (!glide) {
    state_ = LoadState::kError;
    return;
  }

  state_ = LoadState::kLoading;

  LoadOptions opts;
  opts.target_width = static_cast<int>(style().width());
  opts.target_height = static_cast<int>(style().height());

  request_id_ = glide->Load(uri_, [this](const std::string&, std::shared_ptr<Image> img, LoadState s) {
    // Stale callback guard
    if (request_id_ == 0) return;
    loaded_image_ = img;
    state_ = s;
    RequestRedraw();
  }, opts);
}

void ImageWidget::Cancel() {
  if (request_id_ > 0) {
    auto* glide = Glide::Default();
    if (glide) glide->Cancel(request_id_);
    request_id_ = 0;
  }
}

void ImageWidget::Draw(Canvas& canvas) {
  Rect bb = bounds();
  if (bb.width <= 0 || bb.height <= 0) return;

  auto& s = style();

  // Background from style
  Paint bg;
  bg.SetColor(s.background());
  canvas.DrawRect(Rect{0, 0, bb.width, bb.height}, bg);

  // Decide what to draw based on state
  Image* draw_img = nullptr;
  if (state_ == LoadState::kLoaded && loaded_image_) {
    draw_img = loaded_image_.get();
  }

  if (!draw_img) return;

  // ScaleType transform
  ScaleMode mode = s.scale_type();
  float img_w = static_cast<float>(draw_img->width());
  float img_h = static_cast<float>(draw_img->height());
  if (img_w <= 0 || img_h <= 0) return;

  float bw = bb.width, bh = bb.height;
  float sw = bw / img_w, sh = bh / img_h;
  Rect src, dest;

  switch (mode) {
    case ScaleMode::kFillXY:
      dest = Rect{0, 0, bw, bh};
      canvas.DrawImage(*draw_img, dest);
      return;

    case ScaleMode::kCenterCrop: {
      float scale = std::max(sw, sh);
      float sw2 = img_w * scale, sh2 = img_h * scale;
      float sx = (bw - sw2) / 2.0f, sy = (bh - sh2) / 2.0f;
      // Apply gravity offset for crop
      Gravity g = s.scale_gravity();
      if (g == Gravity::kTop) sy = 0;
      else if (g == Gravity::kBottom) sy = bh - sh2;
      else if (g == Gravity::kLeft) sx = 0;
      else if (g == Gravity::kRight) sx = bw - sw2;
      src = Rect{0, 0, img_w, img_h};
      dest = Rect{sx, sy, sw2, sh2};
      canvas.DrawImage(*draw_img, src, dest);
      return;
    }

    case ScaleMode::kCenterInside:
    case ScaleMode::kFitStart:
    case ScaleMode::kFitEnd: {
      float scale = std::min(sw, sh);
      float dw = img_w * scale, dh = img_h * scale;
      float dx = (bw - dw) / 2.0f, dy = (bh - dh) / 2.0f;
      if (mode == ScaleMode::kFitStart) { dx = 0; dy = 0; }
      else if (mode == ScaleMode::kFitEnd) { dx = bw - dw; dy = bh - dh; }
      canvas.DrawImage(*draw_img, Rect{dx, dy, dw, dh});
      return;
    }

    case ScaleMode::kCenter:
    default:
      // Natural size, centered, clipped
      canvas.DrawImage(*draw_img, Rect{(bw - img_w) / 2.0f, (bh - img_h) / 2.0f, img_w, img_h});
      return;
  }
}

}  // namespace native::ui

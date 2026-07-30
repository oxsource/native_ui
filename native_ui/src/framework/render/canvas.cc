#include "canvas.h"

#include "image.h"
#include "paint.h"
#include "path.h"
#include "surface.h"

#include "SkCanvas.h"
#include "SkFont.h"
#include "SkImage.h"
#include "SkPaint.h"
#include "SkRect.h"

namespace native::ui {

static SkColor ToSkColor(Color c) {
  return SkColorSetARGB(c.a, c.r, c.g, c.b);
}

static SkRect ToSkRect(Rect r) {
  return SkRect::MakeXYWH(r.x, r.y, r.width, r.height);
}

static void ApplyPaint(SkPaint& sk_paint, const Paint& paint) {
  sk_paint.setColor(ToSkColor(paint.color()));
  sk_paint.setAntiAlias(paint.anti_alias());
  sk_paint.setStrokeWidth(paint.stroke_width());
  switch (paint.style()) {
    case PaintStyle::kFill:
      sk_paint.setStyle(SkPaint::kFill_Style);
      break;
    case PaintStyle::kStroke:
      sk_paint.setStyle(SkPaint::kStroke_Style);
      break;
    case PaintStyle::kFillAndStroke:
      sk_paint.setStyle(SkPaint::kStrokeAndFill_Style);
      break;
  }
  sk_paint.setAlpha(paint.alpha());
}

class CanvasImpl {
public:
  SkCanvas* sk_canvas = nullptr;
  int save_count = 0;
};

Canvas::Canvas(Surface& surface) : impl_(new CanvasImpl()) {
  impl_->sk_canvas = surface.sk_canvas();
  impl_->save_count = impl_->sk_canvas->save();
}

Canvas::~Canvas() {
  while (impl_->sk_canvas->getSaveCount() > impl_->save_count) {
    impl_->sk_canvas->restore();
  }
  delete impl_;
}

void Canvas::DrawRect(Rect rect, const Paint& paint) {
  SkPaint sk_paint;
  ApplyPaint(sk_paint, paint);
  impl_->sk_canvas->drawRect(ToSkRect(rect), sk_paint);
}

void Canvas::DrawText(const std::string& text, Point pos,
                       const Paint& paint) {
  if (text.empty()) return;
  SkPaint sk_paint;
  ApplyPaint(sk_paint, paint);
  SkFont font;
  impl_->sk_canvas->drawString(text.c_str(), pos.x, pos.y, font, sk_paint);
}

void Canvas::DrawPath(const Path& path, const Paint& paint) {
  SkPaint sk_paint;
  ApplyPaint(sk_paint, paint);
  impl_->sk_canvas->drawPath(*path.sk_path(), sk_paint);
}

void Canvas::DrawImage(const Image& image, Rect dest) {
  SkImage* sk_img = image.sk_image();
  if (sk_img) {
    impl_->sk_canvas->drawImage(sk_img, dest.x, dest.y);
  }
}

void Canvas::DrawImage(const Image& image, Rect src, Rect dest) {
  DrawImage(image, dest);
}

void Canvas::ClipRect(Rect rect) {
  impl_->sk_canvas->clipRect(ToSkRect(rect));
}

void Canvas::Translate(Point offset) {
  impl_->sk_canvas->translate(offset.x, offset.y);
}

void Canvas::Save() {
  impl_->sk_canvas->save();
}

void Canvas::Restore() {
  impl_->sk_canvas->restore();
}

}  // namespace native::ui

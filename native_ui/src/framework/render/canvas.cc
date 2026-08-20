#include "src/framework/render/canvas.h"

#include "src/framework/core/gradient.h"
#include "src/framework/render/font_manager_internal.h"
#include "src/framework/render/image.h"
#include "src/framework/render/paint.h"
#include "src/framework/render/path.h"
#include "src/framework/surface/surface.h"

#include "SkBlurTypes.h"
#include "SkCanvas.h"
#include "SkFont.h"
#include "SkFontTypes.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkRect.h"
#include "SkShader.h"
#include "SkSurface.h"
#include "SkTypeface.h"

namespace native::ui {

static SkColor ToSkColor(Color c) {
  return SkColorSetARGB(c.a, c.r, c.g, c.b);
}

static SkRect ToSkRect(Rect r) {
  return SkRect::MakeXYWH(r.x, r.y, r.width, r.height);
}

static void ApplyPaint(SkPaint& sk_paint, const Paint& paint) {
  auto sk_color = ToSkColor(paint.color());
  sk_paint.setColor(sk_color);
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
  // Combine per-color alpha with paint-level alpha override
  unsigned a = static_cast<unsigned>(SkColorGetA(sk_color)) * paint.alpha() / 255u;
  sk_paint.setAlpha(static_cast<uint8_t>(a));
}

class CanvasImpl {
public:
  SkCanvas* sk_canvas = nullptr;
  int save_count = 0;
};

Canvas::Canvas(Surface& surface) : impl_(new CanvasImpl()) {
  impl_->sk_canvas = static_cast<SkSurface*>(surface.Handle())->getCanvas();
  impl_->save_count = impl_->sk_canvas->save();
}

Canvas::~Canvas() {
  while (impl_->sk_canvas->getSaveCount() > impl_->save_count) {
    impl_->sk_canvas->restore();
  }
  delete impl_;
}

void Canvas::Clear(Color color) {
  impl_->sk_canvas->clear(ToSkColor(color));
}

void Canvas::DrawRect(Rect rect, const Paint& paint) {
  SkPaint sk_paint;
  ApplyPaint(sk_paint, paint);
  impl_->sk_canvas->drawRect(ToSkRect(rect), sk_paint);
}

void Canvas::DrawRoundRect(Rect rect, float radius, const Paint& paint) {
  if (radius <= 0) { DrawRect(rect, paint); return; }
  SkPaint sk_paint;
  ApplyPaint(sk_paint, paint);
  sk_paint.setAntiAlias(true);
  SkRRect rrect = SkRRect::MakeRectXY(ToSkRect(rect), radius, radius);
  impl_->sk_canvas->drawRRect(rrect, sk_paint);
}

void Canvas::DrawGradientRect(Rect rect, const Gradient& gradient) {
  SkRect r = ToSkRect(rect);
  std::vector<SkColor> colors;
  std::vector<SkScalar> positions;
  for (const auto& stop : gradient.stops()) {
    colors.push_back(ToSkColor(stop.color));
    positions.push_back(stop.position);
  }
  if (colors.empty()) return;

  sk_sp<SkShader> shader;
  int n = static_cast<int>(colors.size());
  if (gradient.type() == Gradient::Type::kLinear) {
    SkPoint pts[2] = {
        {gradient.from().x, gradient.from().y},
        {gradient.to().x, gradient.to().y}};
    shader = SkGradientShader::MakeLinear(
        pts, colors.data(), positions.data(), n,
        SkTileMode::kClamp);
  } else {
    shader = SkGradientShader::MakeRadial(
        {gradient.center().x, gradient.center().y},
        gradient.radius(),
        colors.data(), positions.data(), n,
        SkTileMode::kClamp);
  }
  if (!shader) return;

  SkPaint sk_paint;
  sk_paint.setShader(shader);
  impl_->sk_canvas->drawRect(r, sk_paint);
}

void Canvas::DrawShadow(Rect rect, float radius, Point offset, Color color) {
  if (radius <= 0) return;
  SkPaint shadow_paint;
  shadow_paint.setColor(ToSkColor(color));
  auto filter = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, radius);
  if (filter) shadow_paint.setMaskFilter(filter);
  SkRect r = ToSkRect(rect);
  r.offset(offset.x, offset.y);
  impl_->sk_canvas->drawRoundRect(r, radius, radius, shadow_paint);
}

Rect Canvas::MeasureText(const std::string& text, const Font& font) {
  if (text.empty()) return Rect{};
  sk_sp<SkTypeface> tf = FontManagerInternal::ResolveTypeface(font);
  if (!tf) return Rect{};
  const float size = font.size > 0.0f ? font.size : 16.0f;
  SkFont sk_font(tf, size);
  SkRect bounds;
  sk_font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8, &bounds);
  return Rect{bounds.fLeft, bounds.fTop, bounds.width(), bounds.height()};
}

void Canvas::DrawText(const std::string& text, Point pos, const Paint& paint,
                      const Font& font) {
  if (text.empty()) return;
  SkPaint sk_paint;
  ApplyPaint(sk_paint, paint);
  sk_sp<SkTypeface> tf = FontManagerInternal::ResolveTypeface(font);
  if (!tf) return;
  const float size = font.size > 0.0f ? font.size : 16.0f;
  SkFont sk_font(tf, size);
  impl_->sk_canvas->drawString(text.c_str(), pos.x, pos.y, sk_font, sk_paint);
}

void Canvas::DrawText(const std::string& text, Point pos, const Paint& paint,
                      float font_size) {
  Font font;
  font.size = font_size;
  DrawText(text, pos, paint, font);
}

void Canvas::DrawPath(const Path& path, const Paint& paint) {
  SkPaint sk_paint;
  ApplyPaint(sk_paint, paint);
  impl_->sk_canvas->drawPath(*static_cast<SkPath*>(path.Handle()), sk_paint);
}

void Canvas::DrawImage(const Image& image, Rect dest) {
  SkImage* sk_img = static_cast<SkImage*>(image.Handle());
  if (sk_img) {
    impl_->sk_canvas->drawImageRect(sk_img, ToSkRect(dest), SkSamplingOptions());
  }
}

void Canvas::DrawImage1to1(const Image& image, Point dest_top_left) {
  SkImage* sk_img = static_cast<SkImage*>(image.Handle());
  if (sk_img) {
    // drawImage() blits the pixels at their own size with no filtering — the
    // fast path for a pre-scaled image that already matches the target size.
    impl_->sk_canvas->drawImage(sk_img, dest_top_left.x, dest_top_left.y);
  }
}

void Canvas::DrawImage(const Image& image, Rect src, Rect dest) {
  SkImage* sk_img = static_cast<SkImage*>(image.Handle());
  if (sk_img) {
    impl_->sk_canvas->drawImageRect(sk_img, ToSkRect(src), ToSkRect(dest),
                                     SkSamplingOptions(), nullptr,
                                     SkCanvas::kFast_SrcRectConstraint);
  }
}

void Canvas::ClipRect(Rect rect) {
  impl_->sk_canvas->clipRect(ToSkRect(rect));
}

void Canvas::Translate(Point offset) {
  impl_->sk_canvas->translate(offset.x, offset.y);
}

void Canvas::Scale(float sx, float sy) {
  impl_->sk_canvas->scale(sx, sy);
}

void Canvas::Save() {
  impl_->sk_canvas->save();
}

void Canvas::Restore() {
  impl_->sk_canvas->restore();
}

}  // namespace native::ui

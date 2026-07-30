#include "canvas.h"

#include "gradient.h"
#include "image.h"
#include "paint.h"
#include "path.h"
#include "surface.h"

#include "SkBlurTypes.h"
#include "SkCanvas.h"
#include "SkFont.h"
#include "SkFontMgr.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkRRect.h"
#include "SkRect.h"
#include "SkShader.h"
#include "SkTypeface.h"

#if __APPLE__
#include "ports/SkFontMgr_mac_ct.h"
#endif

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

void Canvas::DrawText(const std::string& text, Point pos,
                       const Paint& paint, float font_size) {
  if (text.empty()) return;
  SkPaint sk_paint;
  ApplyPaint(sk_paint, paint);
#if __APPLE__
  static sk_sp<SkFontMgr> font_mgr = SkFontMgr_New_CoreText(nullptr);
  sk_sp<SkTypeface> tf = font_mgr->matchFamilyStyle(nullptr, SkFontStyle());
  SkFont font(tf, font_size);
#else
  SkFont font(nullptr, font_size);
#endif
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

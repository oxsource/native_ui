#include "src/framework/widgets/button.h"
#include "src/framework/render/canvas.h"
#include "src/framework/widgets/event_types.h"
#include "src/framework/render/paint.h"

#include "SkFont.h"
#include "SkFontMgr.h"
#include "SkFontTypes.h"
#include "SkRect.h"
#include "SkTypeface.h"
#if __APPLE__
#include "ports/SkFontMgr_mac_ct.h"
#endif

namespace native::ui {

void Button::ProcessArg(Label tag) { content_ = std::move(tag.value); }
void Button::ProcessArg(OnClick tag) { on_click_ = std::move(tag.value); }

bool Button::HitTest(Point p) const {
  return bounds().Contains(p);
}

bool Button::OnMouseEvent(const MouseEvent& event) {
  if (!style().enabled()) return false;
  if (!bounds().Contains(event.position)) return false;
  pressed_ = true;
  if (on_click_) on_click_();
  return true;
}

void Button::Draw(Canvas& canvas) {
  auto& s = style();
  Rect bb = bounds();

  // Draw state background
  Color bg_color = pressed_ ? s.pressed_color() : s.normal_color();
  Paint bg;
  bg.SetColor(bg_color);
  float cr = s.corner_radius();
  if (cr > 0) {
    canvas.DrawRoundRect(Rect{0, 0, bb.width, bb.height}, cr, bg);
  } else {
    canvas.DrawRect(Rect{0, 0, bb.width, bb.height}, bg);
  }

  // Draw centered text
  std::string text = watched_prop_ ? watched_prop_->value() : content_;
  if (!text.empty()) {
    Paint fg;
    fg.SetColor(s.text_color());
    float size = s.font_size() > 0.0f ? s.font_size() : 16.0f;

    // Measure text width for horizontal centering
#if __APPLE__
    static sk_sp<SkFontMgr> font_mgr = SkFontMgr_New_CoreText(nullptr);
    sk_sp<SkTypeface> tf = font_mgr->matchFamilyStyle(nullptr, SkFontStyle());
    SkFont sk_font(tf, size);
#else
    SkFont sk_font(nullptr, size);
#endif
    SkRect text_bounds;
    sk_font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8, &text_bounds);
    float tx = (bb.width - text_bounds.width()) / 2.0f - text_bounds.left();
    float ty = (bb.height - size) / 2.0f + size;

    canvas.DrawText(text, Point{tx, ty}, fg, size);
  }

  // Dim if disabled
  if (!s.enabled()) {
    Paint dim;
    dim.SetColor(Color{uint8_t{128}, uint8_t{128}, uint8_t{128}, uint8_t{128}});
    canvas.DrawRect(Rect{0, 0, bb.width, bb.height}, dim);
  }
}

}  // namespace native::ui

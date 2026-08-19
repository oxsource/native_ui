#include "src/framework/widgets/text.h"
#include "src/framework/render/canvas.h"
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

void Text::ProcessArg(Content tag) { content_ = std::move(tag.value); }

void Text::Watch(Property<std::string>& prop) {
  watched_prop_ = &prop;
  Widget::Watch(prop);
}

void Text::Draw(Canvas& canvas) {
  std::string text = watched_prop_ ? watched_prop_->value() : content_;
  if (text.empty()) return;

  auto& s = style();
  Rect bb = bounds();

  // Background rect from style
  Paint bg;
  bg.SetColor(s.background());
  canvas.DrawRect(Rect{0, 0, bb.width, bb.height}, bg);

  // Text paint from style
  Paint paint;
  paint.SetColor(s.text_color());

  // Font size from style
  float size = s.font_size() > 0.0f ? s.font_size() : 16.0f;

  // Measure text for centering
#if __APPLE__
  static sk_sp<SkFontMgr> font_mgr = SkFontMgr_New_CoreText(nullptr);
  sk_sp<SkTypeface> tf = font_mgr->matchFamilyStyle(nullptr, SkFontStyle());
  SkFont sk_font(tf, size);
#else
  SkFont sk_font(nullptr, size);
#endif
  SkRect text_bounds;
  sk_font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8, &text_bounds);

  // Horizontal alignment
  TextAlign align = s.text_align();
  float tx;
  if (align == TextAlign::kCenter) {
    tx = (bb.width - text_bounds.width()) / 2.0f - text_bounds.left();
  } else if (align == TextAlign::kRight) {
    tx = bb.width - text_bounds.width() - text_bounds.left() - 8;
  } else {
    tx = 8;  // kLeft
  }

  // Vertical center
  float ty = (bb.height - size) / 2.0f + size;

  canvas.DrawText(text, Point{tx, ty}, paint, size);
}

}  // namespace native::ui

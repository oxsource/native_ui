#include "src/framework/widgets/text.h"
#include "src/framework/render/canvas.h"
#include "src/framework/render/paint.h"

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

  // Font descriptor from style (FR-003): family, weight, size.
  float size = s.font_size() > 0.0f ? s.font_size() : 16.0f;
  Font font;
  font.family = s.font_family();
  font.weight = s.font_weight();
  font.size = size;

  // Measure with the same resolved font used for drawing (FR-008).
  Rect text_bounds = canvas.MeasureText(text, font);

  // Horizontal alignment
  TextAlign align = s.text_align();
  float tx;
  if (align == TextAlign::kCenter) {
    tx = (bb.width - text_bounds.width) / 2.0f - text_bounds.x;
  } else if (align == TextAlign::kRight) {
    tx = bb.width - text_bounds.width - text_bounds.x - 8;
  } else {
    tx = 8;  // kLeft
  }

  // Vertical center
  float ty = (bb.height - size) / 2.0f + size;

  canvas.DrawText(text, Point{tx, ty}, paint, font);
}

}  // namespace native::ui

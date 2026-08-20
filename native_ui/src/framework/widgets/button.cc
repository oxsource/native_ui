#include "src/framework/widgets/button.h"
#include "src/framework/render/canvas.h"
#include "src/framework/widgets/event_types.h"
#include "src/framework/render/paint.h"

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

    // Font descriptor from style (FR-003); same typeface measures & draws.
    Font font;
    font.family = s.font_family();
    font.weight = s.font_weight();
    font.size = size;

    Rect text_bounds = canvas.MeasureText(text, font);
    float tx = (bb.width - text_bounds.width) / 2.0f - text_bounds.x;
    float ty = (bb.height - size) / 2.0f + size;

    canvas.DrawText(text, Point{tx, ty}, fg, font);
  }

  // Dim if disabled
  if (!s.enabled()) {
    Paint dim;
    dim.SetColor(Color{uint8_t{128}, uint8_t{128}, uint8_t{128}, uint8_t{128}});
    canvas.DrawRect(Rect{0, 0, bb.width, bb.height}, dim);
  }
}

}  // namespace native::ui

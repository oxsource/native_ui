#include "button.h"
#include "canvas.h"
#include "event_types.h"
#include "paint.h"

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

  // Draw state background color (overrides the default Text background)
  Color bg_color = pressed_ ? s.pressed_color() : s.normal_color();
  Paint bg;
  bg.SetColor(bg_color);
  canvas.DrawRect(Rect{0, 0, bb.width, bb.height}, bg);

  // Draw text label (inherits FontSize, TextColor, etc. from Text)
  std::string text = watched_prop_ ? watched_prop_->value() : content_;
  if (!text.empty()) {
    Paint fg;
    fg.SetColor(s.text_color());
    float size = s.font_size() > 0.0f ? s.font_size() : 16.0f;
    canvas.DrawText(text, Point{bb.width / 2.0f, bb.height / 2.0f}, fg, size);
  }

  // Dim if disabled
  if (!s.enabled()) {
    Paint dim;
    dim.SetColor(Color{uint8_t{128}, uint8_t{128}, uint8_t{128}, uint8_t{128}});
    canvas.DrawRect(Rect{0, 0, bb.width, bb.height}, dim);
  }
}

}  // namespace native::ui

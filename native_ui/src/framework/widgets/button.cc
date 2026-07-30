#include "button.h"
#include "canvas.h"
#include "event_types.h"
#include "paint.h"

namespace native::ui {

void Button::ProcessArg(Label tag) { label_ = std::move(tag.value); }
void Button::ProcessArg(OnClick tag) { on_click_ = std::move(tag.value); }
void Button::ProcessArg(Id tag) { SetId(std::move(tag.value)); }

void Button::Watch(Property<std::string>& prop) {
  watched_prop_ = &prop;
  Widget::Watch(prop);
}

bool Button::HitTest(Point p) const {
  return bounds().Contains(p);
}

bool Button::OnMouseEvent(const MouseEvent& event) {
  if (!bounds().Contains(event.position)) return false;
  if (on_click_) on_click_();
  return true;
}

void Button::Draw(Canvas& canvas) {
  std::string text = watched_prop_ ? watched_prop_->value() : label_;
  Rect bb = bounds();

  Paint bg;
  bg.SetColor(Color{uint8_t{200}, uint8_t{200}, uint8_t{200}, uint8_t{255}});
  canvas.DrawRect(Rect{0, 0, bb.width, bb.height}, bg);

  if (text.empty()) return;

  Paint fg;
  fg.SetColor(kBlack);
  canvas.DrawText(text, Point{bb.width / 2.0f, bb.height / 2.0f}, fg);
}

}  // namespace native::ui

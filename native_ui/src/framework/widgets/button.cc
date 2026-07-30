#include "button.h"
#include "canvas.h"
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

void Button::Draw(Canvas& canvas) {
  std::string text = watched_prop_ ? watched_prop_->value() : label_;
  Rect bb = bounds();

  Paint bg;
  bg.SetColor(Color{uint8_t{200}, uint8_t{200}, uint8_t{200}, uint8_t{255}});
  canvas.DrawRect(bb, bg);

  if (text.empty()) return;

  Paint fg;
  fg.SetColor(kBlack);
  float cx = bb.x + bb.width / 2.0f;
  float cy = bb.y + bb.height / 2.0f;
  canvas.DrawText(text, Point{cx, cy}, fg);
}

}  // namespace native::ui

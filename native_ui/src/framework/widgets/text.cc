#include "text.h"
#include "canvas.h"
#include "paint.h"

namespace native::ui {

void Text::ProcessArg(Content tag) { content_ = std::move(tag.value); }

void Text::ProcessArg(FontSize tag) {
  font_size_ = tag.value > 0.0f ? tag.value : 1.0f;
}

void Text::ProcessArg(Color tag) { color_ = tag; }

void Text::ProcessArg(Id tag) { SetId(std::move(tag.value)); }

void Text::Watch(Property<std::string>& prop) {
  watched_prop_ = &prop;
  Widget::Watch(prop);
}

void Text::Draw(Canvas& canvas) {
  std::string text = watched_prop_ ? watched_prop_->value() : content_;
  Rect bb = bounds();
  Paint bg;
  bg.SetColor(Color{uint8_t{220}, uint8_t{230}, uint8_t{250}, uint8_t{255}});
  canvas.DrawRect(Rect{0, 0, bb.width, bb.height}, bg);
  if (text.empty()) return;
  Paint paint;
  paint.SetColor(color_);
  canvas.DrawText(text, Point{8, bb.height / 2.0f}, paint);
}

}  // namespace native::ui

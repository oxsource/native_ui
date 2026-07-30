#include "text.h"
#include "canvas.h"
#include "paint.h"

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
  canvas.DrawText(text, Point{8, bb.height / 2.0f}, paint, size);
}

}  // namespace native::ui

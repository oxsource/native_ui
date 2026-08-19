#pragma once

#include <functional>
#include <string>

#include "src/framework/widgets/text.h"

namespace native::ui {

struct Label {
  std::string value;
};

struct OnClick {
  std::function<void()> value;
};

struct NormalColor { Color value; };
struct PressedColor { Color value; };

class Button : public Text {
public:
  template <typename... Args>
  explicit Button(Args&&... args) {
    (ProcessArg(std::forward<Args>(args)), ...);
  }

  using Widget::ProcessArg;  // base style tags
  using Text::ProcessArg;    // text style tags

  bool HitTest(Point p) const;
  bool OnMouseEvent(const MouseEvent& event) override;
  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(Label tag);
  void ProcessArg(OnClick tag);
  void ProcessArg(NormalColor tag) { style_.setNormalColor(tag.value); }
  void ProcessArg(PressedColor tag) { style_.setPressedColor(tag.value); }
  void ProcessArg(Id tag) { SetId(std::move(tag.value)); }

  std::function<void()> on_click_;
  bool pressed_ = false;
};

}  // namespace native::ui

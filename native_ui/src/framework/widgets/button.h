#pragma once

#include <functional>
#include <string>

#include "rect.h"
#include "widget.h"

namespace native::ui {

struct Label {
  std::string value;
};

struct OnClick {
  std::function<void()> value;
};

class Button : public Widget {
public:
  template <typename... Args>
  explicit Button(Args&&... args) {
    (ProcessArg(std::forward<Args>(args)), ...);
  }

  bool HitTest(Point p) const;
  void Watch(Property<std::string>& prop);
  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(Label tag);
  void ProcessArg(OnClick tag);
  void ProcessArg(Id tag);

  std::string label_;
  std::function<void()> on_click_;
  Property<std::string>* watched_prop_ = nullptr;
};

}  // namespace native::ui

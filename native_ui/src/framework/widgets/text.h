#pragma once

#include <string>

#include "color.h"
#include "widget.h"

namespace native::ui {

struct Content {
  std::string value;
};

struct FontSize {
  float value;
};

class Text : public Widget {
public:
  template <typename... Args>
  explicit Text(Args&&... args) {
    (ProcessArg(std::forward<Args>(args)), ...);
  }

  void Watch(Property<std::string>& prop);
  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(Content tag);
  void ProcessArg(FontSize tag);
  void ProcessArg(Color tag);
  void ProcessArg(Id tag);

  std::string content_;
  float font_size_ = 16.0f;
  Color color_ = kBlack;
  Property<std::string>* watched_prop_ = nullptr;
};

}  // namespace native::ui

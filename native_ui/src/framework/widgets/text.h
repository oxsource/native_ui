#pragma once

#include <string>

#include "src/framework/widgets/widget.h"
#include "src/framework/widgets/style.h"

namespace native::ui {

struct Content {
  std::string value;
};

// Text style tags — delegate to Style::setXxx
struct FontSize { float value; };
struct TextColor { Color value; };
struct FontFamily { std::string value; };
struct FontWeight { int value; };
struct LineHeight { float value; };
struct MaxLines { int value; };

class Text : public Widget {
public:
  template <typename... Args>
  explicit Text(Args&&... args) {
    (ProcessArg(std::forward<Args>(args)), ...);
  }

  void Watch(Property<std::string>& prop);
  void Draw(Canvas& canvas) override;

protected:
  using Widget::ProcessArg;  // bring in base style tags (accessible by Button)

  // Non-style tags
  void ProcessArg(Content tag);

  // Style-delegating tags
  void ProcessArg(FontSize tag)     { style_.setFontSize(tag.value); }
  void ProcessArg(TextColor tag)    { style_.setTextColor(tag.value); }
  void ProcessArg(FontFamily tag)   { style_.setFontFamily(tag.value); }
  void ProcessArg(FontWeight tag)   { style_.setFontWeight(tag.value); }
  void ProcessArg(LineHeight tag)   { style_.setLineHeight(tag.value); }
  void ProcessArg(MaxLines tag)     { style_.setMaxLines(tag.value); }
  void ProcessArg(TextAlign v)    { style_.setTextAlign(v); }
  void ProcessArg(TextDecoration v) { style_.setTextDecoration(v); }
  void ProcessArg(Id tag)           { SetId(std::move(tag.value)); }

protected:
  std::string content_;  // non-style: text content
  Property<std::string>* watched_prop_ = nullptr;
};

}  // namespace native::ui

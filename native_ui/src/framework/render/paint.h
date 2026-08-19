#pragma once

#include <cstdint>

#include "src/framework/core/color.h"

namespace native::ui {

enum class PaintStyle {
  kFill,
  kStroke,
  kFillAndStroke,
};

class Paint {
public:
  Paint& SetColor(Color color) {
    color_ = color;
    return *this;
  }

  Paint& SetAntiAlias(bool enabled) {
    anti_alias_ = enabled;
    return *this;
  }

  Paint& SetStrokeWidth(float width) {
    stroke_width_ = width;
    return *this;
  }

  Paint& SetStyle(PaintStyle style) {
    style_ = style;
    return *this;
  }

  Paint& SetAlpha(uint8_t alpha) {
    alpha_ = alpha;
    return *this;
  }

  Color color() const { return color_; }
  bool anti_alias() const { return anti_alias_; }
  float stroke_width() const { return stroke_width_; }
  PaintStyle style() const { return style_; }
  uint8_t alpha() const { return alpha_; }

private:
  Color color_{0, 0, 0, 255};
  bool anti_alias_ = false;
  float stroke_width_ = 0.0f;
  PaintStyle style_ = PaintStyle::kFill;
  uint8_t alpha_ = 255;
};

}  // namespace native::ui

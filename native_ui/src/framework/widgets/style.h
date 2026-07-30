#pragma once

#include <string>

#include "color.h"
#include "edge_insets.h"
#include "gradient.h"
#include "point.h"

namespace native::ui {

// Forward declarations for tag types (defined in widget headers)
// Style uses primitive types directly instead of tag types

enum class StylePriority : int {
  kGlobal   = 100,
  kTheme    = 200,
  kClass    = 300,
  kInstance = 400,
  kExplicit = 500,
};

enum class TextAlign { kLeft, kCenter, kRight };
enum class TextDecoration { kNone, kUnderline, kLineThrough };
enum class ScaleMode { kCenter, kCenterCrop, kCenterInside, kFitStart, kFitEnd, kFillXY };
enum class Gravity { kTop, kBottom, kLeft, kRight, kCenter };

class Style {
public:
  Style();

  // -- Priority --
  Style& setPriority(StylePriority p);
  StylePriority priority() const;

  // -- Common widget properties --
  Style& setWidth(float v);
  Style& setHeight(float v);
  Style& setMinWidth(float v);
  Style& setMaxWidth(float v);
  Style& setPadding(EdgeInsets v);
  Style& setBackground(Color v);
  Style& setBackgroundGradient(Gradient v);
  Style& setEnabled(bool v);
  Style& setVisible(bool v);
  Style& setOpacity(float v);
  Style& setCornerRadius(float v);
  Style& setBorderWidth(float v);
  Style& setBorderColor(Color v);
  Style& setShadowOffset(Point v);
  Style& setShadowRadius(float v);
  Style& setShadowColor(Color v);

  // -- Text properties --
  Style& setFontSize(float v);
  Style& setTextColor(Color v);
  Style& setTextAlign(TextAlign v);
  Style& setFontFamily(const std::string& v);
  Style& setFontWeight(int v);
  Style& setLineHeight(float v);
  Style& setMaxLines(int v);
  Style& setTextDecoration(TextDecoration v);

  // -- Image properties --
  Style& setScaleType(ScaleMode v);
  Style& setScaleGravity(Gravity v);
  Style& setPlaceholder(const std::string& v);
  Style& setErrorImage(const std::string& v);

  // -- Global default --
  static void SetDefault(const Style& s);
  static const Style& Default();

private:
  friend Style Merge(const Style& base, const Style& overlay);

#define PROP(T, name) T name##_; bool name##_set_ = false;

  struct Data {
    PROP(float, width)
    PROP(float, height)
    PROP(float, min_width)
    PROP(float, max_width)
    PROP(EdgeInsets, padding)
    PROP(Color, background)
    PROP(Gradient, background_gradient)
    PROP(bool, enabled)
    PROP(bool, visible)
    PROP(float, opacity)
    PROP(float, corner_radius)
    PROP(float, border_width)
    PROP(Color, border_color)
    PROP(Point, shadow_offset)
    PROP(float, shadow_radius)
    PROP(Color, shadow_color)
    PROP(float, font_size)
    PROP(Color, text_color)
    PROP(TextAlign, text_align)
    PROP(std::string, font_family)
    PROP(int, font_weight)
    PROP(float, line_height)
    PROP(int, max_lines)
    PROP(TextDecoration, text_decoration)
    PROP(ScaleMode, scale_type)
    PROP(Gravity, scale_gravity)
    PROP(std::string, placeholder)
    PROP(std::string, error_image)
  };

#undef PROP

  Data data_;
  StylePriority priority_ = StylePriority::kGlobal;
};

Style Merge(const Style& base, const Style& overlay);

}  // namespace native::ui

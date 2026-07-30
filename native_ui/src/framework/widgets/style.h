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

  // -- Button properties --
  Style& setNormalColor(Color v);
  Style& setPressedColor(Color v);

  // -- Global default --
  static void SetDefault(const Style& s);
  static const Style& Default();

  // -- Getters (read values, is_set indicates if explicitly set) --
  float width() const { return data_.width_; }
  float height() const { return data_.height_; }
  float min_width() const { return data_.min_width_; }
  float max_width() const { return data_.max_width_; }
  EdgeInsets padding() const { return data_.padding_; }
  Color background() const { return data_.background_; }
  const Gradient& background_gradient() const { return data_.background_gradient_; }
  bool enabled() const { return data_.enabled_; }
  bool visible() const { return data_.visible_; }
  float opacity() const { return data_.opacity_; }
  float corner_radius() const { return data_.corner_radius_; }
  float border_width() const { return data_.border_width_; }
  Color border_color() const { return data_.border_color_; }
  Point shadow_offset() const { return data_.shadow_offset_; }
  float shadow_radius() const { return data_.shadow_radius_; }
  Color shadow_color() const { return data_.shadow_color_; }
  float font_size() const { return data_.font_size_; }
  Color text_color() const { return data_.text_color_; }
  TextAlign text_align() const { return data_.text_align_; }
  const std::string& font_family() const { return data_.font_family_; }
  int font_weight() const { return data_.font_weight_; }
  float line_height() const { return data_.line_height_; }
  int max_lines() const { return data_.max_lines_; }
  TextDecoration text_decoration() const { return data_.text_decoration_; }
  ScaleMode scale_type() const { return data_.scale_type_; }
  Gravity scale_gravity() const { return data_.scale_gravity_; }
  const std::string& placeholder() const { return data_.placeholder_; }
  const std::string& error_image() const { return data_.error_image_; }
  Color normal_color() const { return data_.normal_color_; }
  Color pressed_color() const { return data_.pressed_color_; }

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
    PROP(Color, normal_color)
    PROP(Color, pressed_color)
  };

#undef PROP

  Data data_;
  StylePriority priority_ = StylePriority::kGlobal;
};

Style Merge(const Style& base, const Style& overlay);

}  // namespace native::ui

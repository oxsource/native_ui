#include "style.h"

#include <mutex>

namespace native::ui {

static Style g_default_style;
static std::once_flag g_default_init;

Style::Style() {
  std::call_once(g_default_init, []{});
  data_ = g_default_style.data_;
  priority_ = g_default_style.priority_;
}

void Style::SetDefault(const Style& s) {
  g_default_style = s;
  g_default_style.priority_ = StylePriority::kGlobal;
}

const Style& Style::Default() { return g_default_style; }

Style& Style::setPriority(StylePriority p) { priority_ = p; return *this; }
StylePriority Style::priority() const { return priority_; }

Style& Style::setWidth(float v) { data_.width_ = v; data_.width_set_ = true; return *this; }
Style& Style::setHeight(float v) { data_.height_ = v; data_.height_set_ = true; return *this; }
Style& Style::setMinWidth(float v) { data_.min_width_ = v; data_.min_width_set_ = true; return *this; }
Style& Style::setMaxWidth(float v) { data_.max_width_ = v; data_.max_width_set_ = true; return *this; }
Style& Style::setPadding(EdgeInsets v) { data_.padding_ = v; data_.padding_set_ = true; return *this; }
Style& Style::setBackground(Color v) { data_.background_ = v; data_.background_set_ = true; return *this; }
Style& Style::setBackgroundGradient(Gradient v) { data_.background_gradient_ = v; data_.background_gradient_set_ = true; return *this; }
Style& Style::setEnabled(bool v) { data_.enabled_ = v; data_.enabled_set_ = true; return *this; }
Style& Style::setVisible(bool v) { data_.visible_ = v; data_.visible_set_ = true; return *this; }
Style& Style::setOpacity(float v) { data_.opacity_ = v; data_.opacity_set_ = true; return *this; }
Style& Style::setCornerRadius(float v) { data_.corner_radius_ = v; data_.corner_radius_set_ = true; return *this; }
Style& Style::setBorderWidth(float v) { data_.border_width_ = v; data_.border_width_set_ = true; return *this; }
Style& Style::setBorderColor(Color v) { data_.border_color_ = v; data_.border_color_set_ = true; return *this; }
Style& Style::setShadowOffset(Point v) { data_.shadow_offset_ = v; data_.shadow_offset_set_ = true; return *this; }
Style& Style::setShadowRadius(float v) { data_.shadow_radius_ = v; data_.shadow_radius_set_ = true; return *this; }
Style& Style::setShadowColor(Color v) { data_.shadow_color_ = v; data_.shadow_color_set_ = true; return *this; }
Style& Style::setFontSize(float v) { data_.font_size_ = v; data_.font_size_set_ = true; return *this; }
Style& Style::setTextColor(Color v) { data_.text_color_ = v; data_.text_color_set_ = true; return *this; }
Style& Style::setTextAlign(TextAlign v) { data_.text_align_ = v; data_.text_align_set_ = true; return *this; }
Style& Style::setFontFamily(const std::string& v) { data_.font_family_ = v; data_.font_family_set_ = true; return *this; }
Style& Style::setFontWeight(int v) { data_.font_weight_ = v; data_.font_weight_set_ = true; return *this; }
Style& Style::setLineHeight(float v) { data_.line_height_ = v; data_.line_height_set_ = true; return *this; }
Style& Style::setMaxLines(int v) { data_.max_lines_ = v; data_.max_lines_set_ = true; return *this; }
Style& Style::setTextDecoration(TextDecoration v) { data_.text_decoration_ = v; data_.text_decoration_set_ = true; return *this; }
Style& Style::setScaleType(ScaleMode v) { data_.scale_type_ = v; data_.scale_type_set_ = true; return *this; }
Style& Style::setScaleGravity(Gravity v) { data_.scale_gravity_ = v; data_.scale_gravity_set_ = true; return *this; }
Style& Style::setPlaceholder(const std::string& v) { data_.placeholder_ = v; data_.placeholder_set_ = true; return *this; }
Style& Style::setErrorImage(const std::string& v) { data_.error_image_ = v; data_.error_image_set_ = true; return *this; }
Style& Style::setNormalColor(Color v) { data_.normal_color_ = v; data_.normal_color_set_ = true; return *this; }
Style& Style::setPressedColor(Color v) { data_.pressed_color_ = v; data_.pressed_color_set_ = true; return *this; }

// ── Merge ──

#define MERGE(field)                                    \
  if (overlay.data_.field##_set_ &&                     \
      overlay.priority_ >= base.priority_) {            \
    result.data_.field##_ = overlay.data_.field##_;     \
    result.data_.field##_set_ = true;                   \
  }

Style Merge(const Style& base, const Style& overlay) {
  Style result = base;
  result.priority_ = overlay.priority_ >= base.priority_
      ? overlay.priority_ : base.priority_;

  MERGE(width)       MERGE(height)
  MERGE(min_width)   MERGE(max_width)
  MERGE(padding)     MERGE(background)
  MERGE(background_gradient)
  MERGE(enabled)     MERGE(visible)
  MERGE(opacity)     MERGE(corner_radius)
  MERGE(border_width) MERGE(border_color)
  MERGE(shadow_offset) MERGE(shadow_radius)
  MERGE(shadow_color)
  MERGE(font_size)   MERGE(text_color)
  MERGE(text_align)  MERGE(font_family)
  MERGE(font_weight) MERGE(line_height)
  MERGE(max_lines)   MERGE(text_decoration)
  MERGE(scale_type)  MERGE(scale_gravity)
  MERGE(placeholder) MERGE(error_image)
  MERGE(normal_color) MERGE(pressed_color)

  return result;
}

#undef MERGE

}  // namespace native::ui

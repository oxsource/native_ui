#include "src/framework/core/rect.h"

#include "src/framework/core/edge_insets.h"
#include <algorithm>

namespace native::ui {

bool Rect::Contains(Point p) const {
  return p.x >= x && p.x <= x + width &&
         p.y >= y && p.y <= y + height;
}

Rect Rect::Intersect(Rect other) const {
  float l = std::max(x, other.x);
  float t = std::max(y, other.y);
  float r = std::min(x + width, other.x + other.width);
  float b = std::min(y + height, other.y + other.height);
  if (l < r && t < b) {
    return {l, t, r - l, b - t};
  }
  return {};
}

Rect Rect::Union(Rect other) const {
  float l = std::min(x, other.x);
  float t = std::min(y, other.y);
  float r = std::max(x + width, other.x + other.width);
  float b = std::max(y + height, other.y + other.height);
  return {l, t, r - l, b - t};
}

Rect Rect::Inset(EdgeInsets insets) const {
  return {x + insets.left, y + insets.top,
          width - insets.left - insets.right,
          height - insets.top - insets.bottom};
}

Rect Rect::Offset(Point offset) const {
  return {x + offset.x, y + offset.y, width, height};
}

}  // namespace native::ui

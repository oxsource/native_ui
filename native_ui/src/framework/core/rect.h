#pragma once

#include "src/framework/core/point.h"

namespace native::ui {

struct EdgeInsets;

struct Rect {
  float x = 0;
  float y = 0;
  float width = 0;
  float height = 0;

  bool Contains(Point p) const;
  Rect Intersect(Rect other) const;
  Rect Union(Rect other) const;
  Rect Inset(EdgeInsets insets) const;
  Rect Offset(Point offset) const;
};

}  // namespace native::ui

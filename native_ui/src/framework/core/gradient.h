#pragma once

#include <vector>

#include "src/framework/core/color.h"
#include "src/framework/core/point.h"

namespace native::ui {

struct ColorStop {
  float position;
  Color color;
};

class Gradient {
public:
  static Gradient Linear(Point from, Point to,
                         std::vector<ColorStop> stops);
  static Gradient Radial(Point center, float radius,
                         std::vector<ColorStop> stops);

  enum class Type { kLinear, kRadial };

  Type type() const { return type_; }
  const std::vector<ColorStop>& stops() const { return stops_; }

  // Linear-specific
  Point from() const { return from_; }
  Point to() const { return to_; }

  // Radial-specific
  Point center() const { return center_; }
  float radius() const { return radius_; }

public:
  Gradient() = default;

private:

  Type type_;
  std::vector<ColorStop> stops_;
  Point from_, to_;
  Point center_;
  float radius_ = 0;
};

}  // namespace native::ui

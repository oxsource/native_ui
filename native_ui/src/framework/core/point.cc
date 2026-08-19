#include "src/framework/core/point.h"

#include <cmath>

namespace native::ui {

Point Point::operator+(Point other) const {
  return {x + other.x, y + other.y};
}

Point Point::operator-(Point other) const {
  return {x - other.x, y - other.y};
}

float Point::DistanceTo(Point other) const {
  float dx = x - other.x;
  float dy = y - other.y;
  return std::sqrt(dx * dx + dy * dy);
}

}  // namespace native::ui

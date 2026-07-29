#pragma once

namespace native::ui {

struct Point {
  float x = 0;
  float y = 0;

  Point operator+(Point other) const;
  Point operator-(Point other) const;
  float DistanceTo(Point other) const;
};

}  // namespace native::ui

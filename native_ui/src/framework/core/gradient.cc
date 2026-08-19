#include "src/framework/core/gradient.h"

namespace native::ui {

Gradient Gradient::Linear(Point from, Point to,
                          std::vector<ColorStop> stops) {
  Gradient g;
  g.type_ = Type::kLinear;
  g.stops_ = std::move(stops);
  g.from_ = from;
  g.to_ = to;
  return g;
}

Gradient Gradient::Radial(Point center, float radius,
                          std::vector<ColorStop> stops) {
  Gradient g;
  g.type_ = Type::kRadial;
  g.stops_ = std::move(stops);
  g.center_ = center;
  g.radius_ = radius;
  return g;
}

}  // namespace native::ui

#include "path.h"

#include "SkPath.h"

namespace native::ui {

class PathImpl {
public:
  SkPath sk_path;
};

Path::Path() : impl_(new PathImpl()) {}

Path::~Path() { delete impl_; }

Path::Path(Path&& other) noexcept : impl_(other.impl_) {
  other.impl_ = nullptr;
}

Path& Path::operator=(Path&& other) noexcept {
  delete impl_;
  impl_ = other.impl_;
  other.impl_ = nullptr;
  return *this;
}

Path& Path::MoveTo(Point p) {
  impl_->sk_path.moveTo(p.x, p.y);
  return *this;
}

Path& Path::LineTo(Point p) {
  impl_->sk_path.lineTo(p.x, p.y);
  return *this;
}

Path& Path::CubicTo(Point c1, Point c2, Point end_pt) {
  impl_->sk_path.cubicTo(c1.x, c1.y, c2.x, c2.y, end_pt.x, end_pt.y);
  return *this;
}

Path& Path::Close() {
  impl_->sk_path.close();
  return *this;
}

int Path::count_points() const {
  return impl_->sk_path.countPoints();
}

SkPath* Path::sk_path() const {
  return &impl_->sk_path;
}

}  // namespace native::ui

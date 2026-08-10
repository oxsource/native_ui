#pragma once

#include "point.h"

namespace native::ui {

class PathImpl;

class Path {
public:
  Path();
  ~Path();

  Path(const Path&) = delete;
  Path& operator=(const Path&) = delete;
  Path(Path&&) noexcept;
  Path& operator=(Path&&) noexcept;

  Path& MoveTo(Point p);
  Path& LineTo(Point p);
  Path& CubicTo(Point c1, Point c2, Point end_pt);
  Path& Close();

  int count_points() const;

private:
  friend class Canvas;
  // Private (friend-only) renderer handle — opaque, not part of the public API.
  void* Handle() const;

  PathImpl* impl_ = nullptr;
};

}  // namespace native::ui

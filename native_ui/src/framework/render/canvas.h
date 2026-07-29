#pragma once

#include <cstdint>
#include <string>

#include "color.h"
#include "point.h"
#include "rect.h"

namespace native::ui {

class CanvasImpl;
class Paint;
class Path;
class Image;
class Surface;

class Canvas {
public:
  explicit Canvas(Surface& surface);
  ~Canvas();

  void DrawRect(Rect rect, const Paint& paint);
  void DrawText(const std::string& text, Point pos, const Paint& paint);
  void DrawPath(const Path& path, const Paint& paint);
  void DrawImage(const Image& image, Rect dest);
  void DrawImage(const Image& image, Rect src, Rect dest);

  void ClipRect(Rect rect);
  void Translate(Point offset);
  void Save();
  void Restore();

private:
  CanvasImpl* impl_ = nullptr;
};

}  // namespace native::ui

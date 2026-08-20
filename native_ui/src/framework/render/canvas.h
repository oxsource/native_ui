#pragma once

#include <cstdint>
#include <string>

#include "src/framework/core/color.h"
#include "src/framework/core/point.h"
#include "src/framework/core/rect.h"

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

  void Clear(Color color);
  void DrawRect(Rect rect, const Paint& paint);
  void DrawRoundRect(Rect rect, float radius, const Paint& paint);
  void DrawGradientRect(Rect rect, const class Gradient& gradient);
  void DrawShadow(Rect rect, float radius, Point offset, Color color);
  void DrawText(const std::string& text, Point pos, const Paint& paint,
                float font_size = 16.0f);
  void DrawPath(const Path& path, const Paint& paint);
  void DrawImage(const Image& image, Rect dest);
  void DrawImage(const Image& image, Rect src, Rect dest);
  // 1:1 pixel blit with NO resampling: draws `image` at its own size with its
  // top-left at `dest`'s top-left. Skia's drawImage() copies pixels directly,
  // so this is much faster than DrawImage()'s drawImageRect + bilinear sampling
  // when the image already matches the target resolution (e.g. a pre-scaled
  // static background). The caller guarantees image.width() <= dest.width() and
  // image.height() <= dest.height(); pixels outside the image are untouched.
  void DrawImage1to1(const Image& image, Point dest_top_left);

  void ClipRect(Rect rect);
  void Translate(Point offset);
  void Scale(float sx, float sy);
  void Save();
  void Restore();

private:
  CanvasImpl* impl_ = nullptr;
};

}  // namespace native::ui

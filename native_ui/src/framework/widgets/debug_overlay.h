#pragma once

#include <string>

#include "widget.h"

namespace native::ui {

class DebugOverlay : public Widget {
public:
  DebugOverlay();

  void SetRoot(Widget* root) { root_ = root; }
  void Toggle();
  bool visible() const { return visible_; }

  void set_fps(int fps) { fps_ = fps; }
  void set_breadcrumb(const std::string& path) { breadcrumb_ = path; }

  void Draw(Canvas& canvas) override;
  bool OnKeyEvent(const KeyEvent& event) override;

private:
  void DrawLayoutBorders(Canvas& canvas, Widget* widget, int depth);
  void DrawBreadcrumb(Canvas& canvas);
  void DrawFPS(Canvas& canvas);

  Widget* root_ = nullptr;
  bool visible_ = false;
  int fps_ = 0;
  std::string breadcrumb_;
};

}  // namespace native::ui

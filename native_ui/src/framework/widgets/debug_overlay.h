#pragma once

#include "widget.h"

namespace native::ui {

class DebugOverlay : public Widget {
public:
  DebugOverlay();

  void Toggle();
  bool visible() const { return visible_; }

  void set_fps(int fps) { fps_ = fps; }
  void set_breadcrumb(const std::string& path) { breadcrumb_ = path; }

  void Draw(Canvas& canvas) override;

private:
  bool visible_ = false;
  int fps_ = 0;
  std::string breadcrumb_;
};

}  // namespace native::ui

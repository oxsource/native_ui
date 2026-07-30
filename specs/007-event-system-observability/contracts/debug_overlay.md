# DebugOverlay Widget Contract

**Purpose**: Define the DebugOverlay API — toggleable diagnostic widget for development.

## DebugOverlay

```cpp
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
  bool OnKeyEvent(const KeyEvent& event) override;

private:
  void DrawLayoutBorders(Canvas& canvas, Widget* widget);
  void DrawBreadcrumb(Canvas& canvas);
  void DrawFPS(Canvas& canvas);

  bool visible_ = false;
  int fps_ = 0;
  std::string breadcrumb_;
};

}  // namespace native::ui
```

**Contract**:
- `Toggle()` toggles overlay visibility
- `OnKeyEvent` handles F12 (default shortcut) to toggle
- `Draw` draws colored borders around all widgets, FPS counter, and breadcrumb text
- `Draw` is a no-op when `visible_` is false
- `DrawLayoutBorders` walks the widget tree and draws `canvas.DrawRect` borders in distinct colors per depth
- `set_breadcrumb` accepts a string like `"root > Container#header > Text#title"`
- DebugOverlay is compiled out in `NDEBUG` builds — all methods become empty stubs
- DebugOverlay does NOT affect parent layout — it draws on top without modifying bounds

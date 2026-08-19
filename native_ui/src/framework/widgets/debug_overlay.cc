#include "src/framework/widgets/debug_overlay.h"
#include "src/framework/render/canvas.h"
#include "src/framework/widgets/event_types.h"
#include "src/framework/render/paint.h"

#ifndef NDEBUG

namespace native::ui {

DebugOverlay::DebugOverlay() { SetId("__debug_overlay__"); }

void DebugOverlay::Toggle() { visible_ = !visible_; }

static constexpr Color kDepthColors[] = {
    Color{uint8_t{255}, uint8_t{0},   uint8_t{0},   uint8_t{128}},
    Color{uint8_t{0},   uint8_t{255}, uint8_t{0},   uint8_t{128}},
    Color{uint8_t{0},   uint8_t{0},   uint8_t{255}, uint8_t{128}},
    Color{uint8_t{255}, uint8_t{255}, uint8_t{0},   uint8_t{128}},
    Color{uint8_t{255}, uint8_t{0},   uint8_t{255}, uint8_t{128}},
    Color{uint8_t{0},   uint8_t{255}, uint8_t{255}, uint8_t{128}},
};

static constexpr int kNumColors = sizeof(kDepthColors) / sizeof(kDepthColors[0]);

void DebugOverlay::DrawLayoutBorders(Canvas& canvas, Widget* widget, int depth) {
  if (!widget) return;

  // Draw border at current widget
  Paint border;
  border.SetColor(kDepthColors[depth % kNumColors])
        .SetStyle(PaintStyle::kStroke)
        .SetStrokeWidth(1.0f);
  canvas.DrawRect(widget->bounds(), border);

  // Recurse into children
  for (int i = 0; i < widget->ChildCount(); ++i) {
    Widget* child = widget->ChildAt(i);
    if (!child) continue;
    canvas.Save();
    canvas.Translate(Point{child->bounds().x, child->bounds().y});
    DrawLayoutBorders(canvas, child, depth + 1);
    canvas.Restore();
  }
}

void DebugOverlay::DrawBreadcrumb(Canvas& canvas) {
  if (breadcrumb_.empty()) return;
  Paint text_paint;
  text_paint.SetColor(Color{uint8_t{255}, uint8_t{255}, uint8_t{255}, uint8_t{200}});
  canvas.DrawText(breadcrumb_, Point{4, 12}, text_paint);
}

void DebugOverlay::DrawFPS(Canvas& canvas) {
  std::string fps_text = "FPS: " + std::to_string(fps_);
  Paint text_paint;
  text_paint.SetColor(Color{uint8_t{0}, uint8_t{255}, uint8_t{0}, uint8_t{200}});
  canvas.DrawText(fps_text, Point{4, 24}, text_paint);
}

void DebugOverlay::Draw(Canvas& canvas) {
  if (!visible_) return;

  // Draw layout borders starting from root (or self if no root set)
  Widget* walk_root = root_ ? root_ : this;
  DrawLayoutBorders(canvas, walk_root, 0);

  // Draw FPS counter
  DrawFPS(canvas);

  // Draw breadcrumb
  DrawBreadcrumb(canvas);
}

bool DebugOverlay::OnKeyEvent(const KeyEvent& event) {
  // F12 toggles overlay visibility
  if (event.key_code == 0) {  // F12 maps to key_code 0 in MVP
    Toggle();
    return true;
  }
  return false;
}

}  // namespace native::ui

#else  // NDEBUG

namespace native::ui {

DebugOverlay::DebugOverlay() {}
void DebugOverlay::Toggle() {}
void DebugOverlay::DrawLayoutBorders(Canvas&, Widget*, int) {}
void DebugOverlay::DrawBreadcrumb(Canvas&) {}
void DebugOverlay::DrawFPS(Canvas&) {}
void DebugOverlay::Draw(Canvas&) {}
bool DebugOverlay::OnKeyEvent(const KeyEvent&) { return false; }

}  // namespace native::ui

#endif  // NDEBUG

#include "gtest/gtest.h"
#include "canvas.h"
#include "container.h"
#include "debug_overlay.h"
#include "event_types.h"
#include "surface.h"

namespace native::ui {

// ============================================================================
// DebugOverlay Tests
// ============================================================================

TEST(DebugOverlayTest, DefaultNotVisible) {
  DebugOverlay overlay;
  EXPECT_FALSE(overlay.visible());
}

TEST(DebugOverlayTest, Toggle) {
  DebugOverlay overlay;
  overlay.Toggle();
  EXPECT_TRUE(overlay.visible());
  overlay.Toggle();
  EXPECT_FALSE(overlay.visible());
}

TEST(DebugOverlayTest, SetFps) {
  DebugOverlay overlay;
  overlay.set_fps(60);
  // No getter for fps — verify no crash
}

TEST(DebugOverlayTest, DrawWhenHiddenNoCrash) {
  DebugOverlay overlay;
  auto surface = Surface::Create(100, 30);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    overlay.Draw(canvas);
  }
}

TEST(DebugOverlayTest, DrawWhenVisibleNoCrash) {
  DebugOverlay overlay;
  overlay.Toggle();
  overlay.set_fps(60);
  overlay.set_breadcrumb("root > Container#main > Text#title");

  Container root;
  root.SetBounds(Rect{0, 0, 200, 200});
  auto child = std::make_unique<Container>();
  child->SetBounds(Rect{10, 10, 100, 50});
  child->SetId("main");
  root.AddChild(std::move(child));
  overlay.SetRoot(&root);

  auto surface = Surface::Create(200, 200);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    overlay.Draw(canvas);
  }
}

TEST(DebugOverlayTest, SetBreadcrumb) {
  DebugOverlay overlay;
  overlay.set_breadcrumb("root > widget");
  overlay.Toggle();
  auto surface = Surface::Create(100, 30);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    overlay.Draw(canvas);
  }
}

TEST(DebugOverlayTest, KeyEventToggle) {
  DebugOverlay overlay;
  EXPECT_FALSE(overlay.visible());

  // F12 key press
  KeyEvent key;
  key.key_code = 0;
  overlay.OnKeyEvent(key);
  EXPECT_TRUE(overlay.visible());
}

}  // namespace native::ui

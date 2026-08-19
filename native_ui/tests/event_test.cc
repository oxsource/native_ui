#include "gtest/gtest.h"
#include "src/framework/widgets/button.h"
#include "src/framework/widgets/container.h"
#include "src/framework/event/dispatch_result.h"
#include "src/framework/event/event.h"
#include "src/framework/widgets/event_types.h"
#include "src/framework/event/hit_tester.h"
#include "src/framework/widgets/stack.h"

namespace native::ui {

class NullWidget : public Widget {
public:
  void Draw(Canvas&) override {}
};

class TestState : public State {
public:
  Property<int> count{this};
  Property<std::string> name{this};
};

// ============================================================================
// DispatchResult Tests
// ============================================================================

TEST(DispatchResultTest, DefaultValues) {
  DispatchResult r;
  EXPECT_EQ(r.status, DispatchStatus::kNoTarget);
  EXPECT_EQ(r.target, nullptr);
}

TEST(DispatchResultTest, EnumValues) {
  EXPECT_NE(DispatchStatus::kHandled, DispatchStatus::kUnhandled);
  EXPECT_NE(DispatchStatus::kRejected, DispatchStatus::kNoTarget);
}

// ============================================================================
// Event Types Tests
// ============================================================================

TEST(EventTypesTest, MouseEventDefaults) {
  MouseEvent e;
  EXPECT_EQ(e.position.x, 0);
  EXPECT_EQ(e.position.y, 0);
  EXPECT_EQ(e.button, 0);
  EXPECT_EQ(e.modifiers, 0);
}

TEST(EventTypesTest, KeyEventDefaults) {
  KeyEvent e;
  EXPECT_EQ(e.key_code, 0);
  EXPECT_EQ(e.modifiers, 0);
}

TEST(EventTypesTest, MouseButtonValues) {
  EXPECT_EQ(MouseButton::kLeft, 0);
  EXPECT_EQ(MouseButton::kRight, 1);
  EXPECT_EQ(MouseButton::kMiddle, 2);
}

// ============================================================================
// HitTester Tests
// ============================================================================

TEST(HitTesterTest, NullRoot) {
  HitTester tester;
  HitTestResult result = tester.Test(nullptr, Point{10, 10});
  EXPECT_EQ(result.widget, nullptr);
}

TEST(HitTesterTest, SingleWidgetHit) {
  NullWidget w;
  w.SetBounds(Rect{0, 0, 100, 100});
  HitTester tester;
  HitTestResult result = tester.Test(&w, Point{50, 50});
  EXPECT_EQ(result.widget, &w);
  EXPECT_FLOAT_EQ(result.local_pos.x, 50);
  EXPECT_FLOAT_EQ(result.local_pos.y, 50);
}

TEST(HitTesterTest, SingleWidgetMiss) {
  NullWidget w;
  w.SetBounds(Rect{0, 0, 100, 100});
  HitTester tester;
  HitTestResult result = tester.Test(&w, Point{200, 200});
  EXPECT_EQ(result.widget, nullptr);
}

TEST(HitTesterTest, DeepestWidgetFound) {
  Container outer;
  outer.SetBounds(Rect{0, 0, 200, 200});
  auto inner = std::make_unique<NullWidget>();
  inner->SetBounds(Rect{50, 50, 100, 100});
  auto* inner_raw = inner.get();
  outer.AddChild(std::move(inner));

  HitTester tester;
  HitTestResult result = tester.Test(&outer, Point{75, 75});
  // Should find the deepest widget (inner) since point is within its bounds
  EXPECT_EQ(result.widget, inner_raw);
}

TEST(HitTesterTest, NoHitForEmptyArea) {
  Container outer;
  outer.SetBounds(Rect{0, 0, 200, 200});
  auto inner = std::make_unique<NullWidget>();
  inner->SetBounds(Rect{50, 50, 100, 100});
  outer.AddChild(std::move(inner));

  HitTester tester;
  HitTestResult result = tester.Test(&outer, Point{10, 10});
  // Point is in outer but not in inner — deepest is outer
  EXPECT_EQ(result.widget, &outer);
}

// ============================================================================
// EventHub Tests
// ============================================================================

TEST(EventHubTest, PushToNullRoot) {
  EventHub hub;
  MouseEvent event;
  event.position = Point{50, 50};
  DispatchResult result = hub.Push(event);
  EXPECT_EQ(result.status, DispatchStatus::kNoTarget);
}

TEST(EventHubTest, FilterRejectsEvent) {
  EventHub hub;

  // Register filter that rejects events at x < 50
  hub.AddFilter([](const MouseEvent& e) -> bool {
    return e.position.x >= 50;
  });

  MouseEvent event;
  event.position = Point{10, 10};
  DispatchResult result = hub.Push(event);
  EXPECT_EQ(result.status, DispatchStatus::kRejected);
}

TEST(EventHubTest, FilterAllowsEvent) {
  EventHub hub;
  hub.SetRoot(nullptr);  // prevent crash, will return kNoTarget

  hub.AddFilter([](const MouseEvent& e) -> bool {
    return e.position.x >= 50;
  });

  MouseEvent event;
  event.position = Point{100, 10};
  DispatchResult result = hub.Push(event);
  // Filter passes → dispatches → no root → kNoTarget
  EXPECT_NE(result.status, DispatchStatus::kRejected);
}

TEST(EventHubTest, ButtonClickHandled) {
  auto state = std::make_shared<TestState>();
  bool clicked = false;
  auto button = std::make_unique<Button>(Label{"OK"}, OnClick{[&] { clicked = true; }});
  button->SetBounds(Rect{10, 10, 80, 40});
  auto* btn_raw = button.get();

  Container root;
  root.SetBounds(Rect{0, 0, 200, 200});
  root.AddChild(std::move(button));

  EventHub hub;
  hub.SetRoot(&root);

  MouseEvent event;
  event.position = Point{50, 30};  // inside button

  DispatchResult result = hub.Push(event);
  EXPECT_EQ(result.status, DispatchStatus::kHandled);
  EXPECT_EQ(result.target, btn_raw);
  EXPECT_TRUE(clicked);
}

TEST(EventHubTest, ClickOutsideAllWidgets) {
  Container root;
  root.SetBounds(Rect{0, 0, 200, 200});
  auto button = std::make_unique<Button>(Label{"OK"});
  button->SetBounds(Rect{10, 10, 80, 40});
  root.AddChild(std::move(button));

  EventHub hub;
  hub.SetRoot(&root);

  MouseEvent event;
  event.position = Point{300, 300};  // outside root bounds

  // Note: HitTester returns null for points outside root bounds
  // but if root contains the point and no child does, root is deepest
  DispatchResult result = hub.Push(event);
  // HitTester with root-relative: root bounds {0,0,200,200} → 300,300 is outside
  // So HitTester returns null → kNoTarget
  EXPECT_EQ(result.status, DispatchStatus::kNoTarget);
}

TEST(EventHubTest, MultipleFilters) {
  EventHub hub;
  int filter1_called = 0;
  int filter2_called = 0;

  hub.AddFilter([&](const MouseEvent&) -> bool {
    filter1_called++;
    return true;  // allow
  });
  hub.AddFilter([&](const MouseEvent&) -> bool {
    filter2_called++;
    return true;  // allow
  });

  hub.SetRoot(nullptr);
  MouseEvent event;
  hub.Push(event);

  EXPECT_EQ(filter1_called, 1);
  EXPECT_EQ(filter2_called, 1);
}

}  // namespace native::ui

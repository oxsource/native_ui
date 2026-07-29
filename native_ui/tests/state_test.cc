#include "gtest/gtest.h"
#include "state.h"
#include "widget.h"

namespace native::ui {

class TestState : public State {
public:
  Property<int> count{this};
  Property<std::string> name{this};
};

class TestWidget : public Widget {
public:
  void Draw(Canvas&) override {}
};

TEST(StateTest, PropertyNotifyWatcher) {
  auto state = std::make_shared<TestState>();
  TestWidget widget;
  widget.Watch(state->count);

  EXPECT_FALSE(widget.needs_draw());
  state->count = 42;
  state->Flush();
  EXPECT_TRUE(widget.needs_draw());
}

TEST(StateTest, BatchCoalescing) {
  auto state = std::make_shared<TestState>();
  TestWidget widget_count;
  TestWidget widget_name;
  widget_count.Watch(state->count);
  widget_name.Watch(state->name);

  state->count = 1;
  state->count = 2;
  state->name = "hello";

  state->Flush();
  EXPECT_TRUE(widget_count.needs_draw());
  EXPECT_TRUE(widget_name.needs_draw());
}

TEST(StateTest, MultiplePropertiesSameWidget) {
  auto state = std::make_shared<TestState>();
  TestWidget widget;
  widget.Watch(state->count);
  widget.Watch(state->name);

  state->count = 99;
  state->name = "test";
  state->Flush();

  EXPECT_TRUE(widget.needs_draw());
}

TEST(StateTest, NoNotifyAfterDestroy) {
  auto state = std::make_shared<TestState>();
  {
    TestWidget widget;
    widget.Watch(state->count);
    state->count = 10;
  }
  // Widget destroyed, flush should not crash
  state->Flush();
}

}  // namespace native::ui

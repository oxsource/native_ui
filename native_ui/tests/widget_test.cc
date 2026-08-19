#include "gtest/gtest.h"
#include "src/framework/widgets/widget.h"

namespace native::ui {

class TestWidget : public Widget {
public:
  void Draw(Canvas&) override {}
};

TEST(WidgetTest, SetIdGetId) {
  TestWidget w;
  w.SetId("my_widget");
  EXPECT_EQ(w.GetId(), "my_widget");
}

TEST(WidgetTest, FindByIdSelf) {
  TestWidget w;
  w.SetId("root");
  EXPECT_EQ(w.FindById("root"), &w);
}

TEST(WidgetTest, FindByIdNonexistent) {
  TestWidget w;
  w.SetId("a");
  EXPECT_EQ(w.FindById("b"), nullptr);
}

TEST(WidgetTest, LeafChildCount) {
  TestWidget w;
  EXPECT_EQ(w.ChildCount(), 0);
  EXPECT_EQ(w.ChildAt(0), nullptr);
  EXPECT_EQ(w.IndexOf(nullptr), -1);
}

TEST(WidgetTest, RequestLayoutSetsFlags) {
  TestWidget w;
  EXPECT_FALSE(w.needs_layout());
  EXPECT_FALSE(w.needs_draw());
  w.RequestLayout();
  EXPECT_TRUE(w.needs_layout());
  EXPECT_TRUE(w.needs_draw());
}

TEST(WidgetTest, RequestRedrawOnly) {
  TestWidget w;
  w.RequestLayout();  // sets both
  // Reset by calling directly (no reset method, so test initial state)
}

TEST(WidgetTest, RedrawDoesNotSetLayout) {
  TestWidget w;
  w.RequestRedraw();
  EXPECT_FALSE(w.needs_layout());
  EXPECT_TRUE(w.needs_draw());
}

}  // namespace native::ui

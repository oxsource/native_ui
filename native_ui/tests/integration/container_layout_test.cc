#include "gtest/gtest.h"
#include "src/framework/widgets/container.h"
#include "yoga/Yoga.h"

namespace native::ui {

class LayoutWidget : public Widget {
public:
  explicit LayoutWidget(float w, float h) : w_(w), h_(h) {}
  void Draw(Canvas&) override {}

  float width() const { return w_; }
  float height() const { return h_; }

private:
  float w_, h_;
};

TEST(ContainerLayoutTest, RowLayoutWithGap) {
  Container c(Direction{Direction::kRow}, Gap{8}, Padding{12});

  auto child1 = std::make_unique<LayoutWidget>(60, 40);
  auto child2 = std::make_unique<LayoutWidget>(80, 40);
  c.AddChild(std::move(child1));
  c.AddChild(std::move(child2));

  // Access the Container's root YGNodeRef for verification
  // Yoga should have nodes inserted via AddChild
  // Verify children count
  EXPECT_EQ(c.ChildCount(), 2);
}

TEST(ContainerLayoutTest, ColumnLayout) {
  Container c(Direction{Direction::kColumn}, Gap{8});

  auto child1 = std::make_unique<LayoutWidget>(100, 50);
  auto child2 = std::make_unique<LayoutWidget>(100, 30);
  c.AddChild(std::move(child1));
  c.AddChild(std::move(child2));

  EXPECT_EQ(c.ChildCount(), 2);
}

TEST(ContainerLayoutTest, FlexLayoutYogaCalculate) {
  YGNodeRef root = YGNodeNew();
  YGNodeStyleSetFlexDirection(root, YGFlexDirectionRow);
  YGNodeStyleSetPadding(root, YGEdgeAll, 12);
  YGNodeStyleSetGap(root, YGGutterAll, 8);
  YGNodeStyleSetWidth(root, 300);

  YGNodeRef child1 = YGNodeNew();
  YGNodeStyleSetWidth(child1, 60);
  YGNodeStyleSetHeight(child1, 40);
  YGNodeInsertChild(root, child1, 0);

  YGNodeRef child2 = YGNodeNew();
  YGNodeStyleSetWidth(child2, 80);
  YGNodeStyleSetHeight(child2, 40);
  YGNodeInsertChild(root, child2, 1);

  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  // With padding=12 and gap=8:
  // child1: x = 12, y = 12, w = 60, h = 40
  // child2: x = 12 + 60 + 8 = 80, y = 12, w = 80, h = 40
  EXPECT_FLOAT_EQ(YGNodeLayoutGetLeft(child1), 12);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetTop(child1), 12);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetWidth(child1), 60);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetHeight(child1), 40);

  EXPECT_FLOAT_EQ(YGNodeLayoutGetLeft(child2), 80);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetTop(child2), 12);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetWidth(child2), 80);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetHeight(child2), 40);

  // Total width should be at least 12 + 60 + 8 + 80 + 12 = 172
  EXPECT_GE(YGNodeLayoutGetWidth(root), 172);

  YGNodeFreeRecursive(root);
}

}  // namespace native::ui

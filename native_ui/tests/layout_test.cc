#include "gtest/gtest.h"
#include "src/framework/layout/flex_layout.h"

namespace native::ui {

// Helper: create a child node with given size
YGNodeRef MakeChild(float w, float h) {
  auto* node = YGNodeNew();
  YGNodeStyleSetWidth(node, w);
  YGNodeStyleSetHeight(node, h);
  return node;
}

// Helper: free children
void FreeChildren(const std::vector<YGNodeRef>& nodes) {
  for (auto* n : nodes) YGNodeFree(n);
}

TEST(LayoutTest, DirectionRow) {
  FlexLayout layout(Direction{0});
  auto c1 = MakeChild(60, 40);
  auto c2 = MakeChild(80, 40);
  layout.SetChildren({c1, c2});
  auto result = layout.Measure({300, 100});
  layout.Arrange(result, {300, 100});

  EXPECT_FLOAT_EQ(result[0].size.width, 60);
  EXPECT_FLOAT_EQ(result[0].size.height, 40);
  EXPECT_FLOAT_EQ(result[1].size.width, 80);
  EXPECT_FLOAT_EQ(result[1].size.height, 40);

  // Row: c1 at x=0, c2 at x=60 (no gap)
  EXPECT_FLOAT_EQ(result[0].position.x, 0);
  EXPECT_FLOAT_EQ(result[1].position.x, 60);

  FreeChildren({c1, c2});
}

TEST(LayoutTest, DirectionColumn) {
  FlexLayout layout(Direction{1});
  auto c1 = MakeChild(100, 50);
  auto c2 = MakeChild(100, 30);
  layout.SetChildren({c1, c2});
  auto result = layout.Measure({200, 200});
  layout.Arrange(result, {200, 200});

  // Column: c1 at y=0, c2 at y=50 (no gap)
  EXPECT_FLOAT_EQ(result[0].position.y, 0);
  EXPECT_FLOAT_EQ(result[1].position.y, 50);

  FreeChildren({c1, c2});
}

TEST(LayoutTest, JustifyContentCenter) {
  FlexLayout layout(Direction{0}, JustifyContent{JustifyContent::kCenter});
  auto c1 = MakeChild(60, 40);
  auto c2 = MakeChild(60, 40);
  layout.SetChildren({c1, c2});
  auto result = layout.Measure({300, 100});
  layout.Arrange(result, {300, 100});

  // Two 60-wide children in a 300-wide container: total = 120
  // center offset = (300 - 120) / 2 = 90
  EXPECT_FLOAT_EQ(result[0].position.x, 90);
  EXPECT_FLOAT_EQ(result[1].position.x, 150);

  FreeChildren({c1, c2});
}

TEST(LayoutTest, JustifyContentSpaceBetween) {
  FlexLayout layout(Direction{0}, JustifyContent{JustifyContent::kSpaceBetween});
  auto c1 = MakeChild(60, 40);
  auto c2 = MakeChild(60, 40);
  layout.SetChildren({c1, c2});
  auto result = layout.Measure({300, 100});
  layout.Arrange(result, {300, 100});

  // Two 60-wide children, container 300: remaining = 180
  // space-between: first at 0, last at 240, gap = 180
  EXPECT_FLOAT_EQ(result[0].position.x, 0);
  EXPECT_FLOAT_EQ(result[1].position.x, 240);

  FreeChildren({c1, c2});
}

TEST(LayoutTest, AlignItemsStretch) {
  FlexLayout layout(Direction{0}, AlignItems{AlignItems::kStretch});
  auto c1 = YGNodeNew();
  YGNodeStyleSetWidth(c1, 60);
  // Height not set = YGUndefined → stretch to container height
  layout.SetChildren({c1});
  auto result = layout.Measure({300, 100});
  layout.Arrange(result, {300, 100});

  // Stretch: child height should equal container height
  EXPECT_FLOAT_EQ(result[0].size.height, 100);

  YGNodeFree(c1);
}

TEST(LayoutTest, GapSpacing) {
  FlexLayout layout(Direction{0}, Gap{8});
  auto c1 = MakeChild(60, 40);
  auto c2 = MakeChild(80, 40);
  layout.SetChildren({c1, c2});
  auto result = layout.Measure({300, 100});
  layout.Arrange(result, {300, 100});

  // c1 at x=0, gap=8, so c2 at x=60+8=68
  EXPECT_FLOAT_EQ(result[0].position.x, 0);
  EXPECT_FLOAT_EQ(result[1].position.x, 68);

  FreeChildren({c1, c2});
}

TEST(LayoutTest, FlexWrapGap) {
  FlexLayout layout(Direction{0}, Gap{8}, FlexWrap{FlexWrap::kWrap});
  auto c1 = MakeChild(100, 40);
  auto c2 = MakeChild(100, 40);
  layout.SetChildren({c1, c2});
  auto result = layout.Measure({160, 200});
  layout.Arrange(result, {160, 200});

  // Should wrap: c1 on first row, c2 on second row
  EXPECT_EQ(result.size(), 2u);
  EXPECT_FLOAT_EQ(result[0].position.x, 0);
  EXPECT_GT(result[1].position.y, 0);

  FreeChildren({c1, c2});
}

TEST(LayoutTest, MarginSpacing) {
  FlexLayout layout(Direction{0});
  auto c1 = MakeChild(60, 40);
  YGNodeStyleSetMargin(c1, YGEdgeAll, 8);
  auto c2 = MakeChild(80, 40);
  YGNodeStyleSetMargin(c2, YGEdgeAll, 8);
  layout.SetChildren({c1, c2});
  auto result = layout.Measure({300, 100});
  layout.Arrange(result, {300, 100});

  // margin 8: c1 at x=8, c2 at x=8+60+8+8=84 (c1 right margin + c2 left margin = 16)
  EXPECT_FLOAT_EQ(result[0].position.x, 8);
  EXPECT_FLOAT_EQ(result[1].position.x, 84);

  FreeChildren({c1, c2});
}

TEST(LayoutTest, FlexWrap) {
  FlexLayout layout(Direction{0}, FlexWrap{FlexWrap::kWrap});
  auto c1 = MakeChild(200, 40);
  auto c2 = MakeChild(200, 40);
  auto c3 = MakeChild(200, 40);
  layout.SetChildren({c1, c2, c3});
  auto result = layout.Measure({250, 200});
  layout.Arrange(result, {250, 200});

  // Container 250 wide, children 200 each: wrap after 1st child
  // c1 at row 0, y=0; c2 at row 1, y=40
  EXPECT_FLOAT_EQ(result[0].position.y, 0);
  EXPECT_FLOAT_EQ(result[1].position.y, 40);
  EXPECT_FLOAT_EQ(result[2].position.y, 80);

  FreeChildren({c1, c2, c3});
}

TEST(LayoutTest, FlexGrow) {
  FlexLayout layout(Direction{0});
  auto c1 = MakeChild(60, 40);
  YGNodeStyleSetFlexGrow(c1, 1);
  auto c2 = MakeChild(60, 40);
  YGNodeStyleSetFlexGrow(c2, 2);
  layout.SetChildren({c1, c2});
  auto result = layout.Measure({300, 100});
  layout.Arrange(result, {300, 100});

  // Total fixed = 120, remaining = 180
  // c1 grow 1, c2 grow 2: c1 gets 60 extra, c2 gets 120 extra
  // c1 width = 60+60 = 120, c2 width = 60+120 = 180
  EXPECT_FLOAT_EQ(result[0].size.width, 120);
  EXPECT_FLOAT_EQ(result[1].size.width, 180);
  // Total should be 300
  EXPECT_FLOAT_EQ(result[0].size.width + result[1].size.width, 300);

  FreeChildren({c1, c2});
}

TEST(LayoutTest, AlignContentCenter) {
  FlexLayout layout(Direction{0}, FlexWrap{FlexWrap::kWrap}, AlignContent{AlignContent::kCenter});
  auto c1 = MakeChild(200, 40);
  auto c2 = MakeChild(200, 40);
  layout.SetChildren({c1, c2});
  auto result = layout.Measure({250, 200});
  layout.Arrange(result, {250, 200});

  // Two rows of 40px each, container height 200
  // align-content center: both rows vertically centered together
  // Content block = 80px, remaining = 120, offset = 60
  // Row 1 at y=60, Row 2 at y=100
  EXPECT_FLOAT_EQ(result[0].position.y, 60);
  EXPECT_FLOAT_EQ(result[1].position.y, 100);

  FreeChildren({c1, c2});
}

TEST(LayoutTest, EdgeCases) {
  // Empty children — should not crash
  FlexLayout empty(Direction{0});
  auto result = empty.Measure({100, 100});
  EXPECT_TRUE(result.empty());

  // Single child with flex-shrink
  auto c1 = MakeChild(200, 40);
  YGNodeStyleSetFlexShrink(c1, 1);

  FlexLayout shrinkLayout(Direction{0});
  shrinkLayout.SetChildren({c1});
  auto shrinkResult = shrinkLayout.Measure({100, 100});
  shrinkLayout.Arrange(shrinkResult, {100, 100});

  // Child should shrink from 200 to 100
  EXPECT_FLOAT_EQ(shrinkResult[0].size.width, 100);

  YGNodeFree(c1);
}

}  // namespace native::ui

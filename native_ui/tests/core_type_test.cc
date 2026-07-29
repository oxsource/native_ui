#include "gtest/gtest.h"
#include "size.h"
#include "color.h"
#include "edge_insets.h"
#include "rect.h"

namespace native::ui {

// Size tests
TEST(SizeTest, IsEmpty) {
  EXPECT_TRUE((Size{0, 10}.IsEmpty()));
  EXPECT_TRUE((Size{10, 0}.IsEmpty()));
  EXPECT_TRUE((Size{-5, 10}.IsEmpty()));
  EXPECT_FALSE((Size{10, 20}.IsEmpty()));
}

TEST(SizeTest, Equality) {
  EXPECT_TRUE((Size{10, 20} == Size{10, 20}));
  EXPECT_FALSE((Size{10, 20} == Size{30, 40}));
}

// Color tests
TEST(ColorTest, ChannelAccess) {
  Color c{uint8_t{100}, uint8_t{150}, uint8_t{200}, uint8_t{255}};
  EXPECT_EQ(c.r, 100);
  EXPECT_EQ(c.g, 150);
  EXPECT_EQ(c.b, 200);
  EXPECT_EQ(c.a, 255);
}

TEST(ColorTest, ClampOnConstruction) {
  Color c{300, -10, 128, 255};
  EXPECT_EQ(c.r, 255);
  EXPECT_EQ(c.g, 0);
  EXPECT_EQ(c.b, 128);
  EXPECT_EQ(c.a, 255);
}

TEST(ColorTest, NamedConstants) {
  EXPECT_EQ(kRed.r, 255);
  EXPECT_EQ(kRed.g, 0);
  EXPECT_EQ(kGreen.g, 255);
  EXPECT_EQ(kBlue.b, 255);
  EXPECT_EQ(kWhite.r, 255);
  EXPECT_EQ(kWhite.g, 255);
  EXPECT_EQ(kWhite.b, 255);
  EXPECT_EQ(kBlack.r, 0);
  EXPECT_EQ(kTransparent.a, 0);
}

// EdgeInsets tests
TEST(EdgeInsetsTest, All) {
  auto insets = EdgeInsets::All(8);
  EXPECT_FLOAT_EQ(insets.top, 8);
  EXPECT_FLOAT_EQ(insets.left, 8);
  EXPECT_FLOAT_EQ(insets.bottom, 8);
  EXPECT_FLOAT_EQ(insets.right, 8);
}

TEST(EdgeInsetsTest, Symmetric) {
  auto insets = EdgeInsets::Symmetric(10, 20);
  EXPECT_FLOAT_EQ(insets.top, 20);
  EXPECT_FLOAT_EQ(insets.left, 10);
  EXPECT_FLOAT_EQ(insets.bottom, 20);
  EXPECT_FLOAT_EQ(insets.right, 10);
}

TEST(EdgeInsetsTest, Only) {
  auto insets = EdgeInsets::Only(1, 2, 3, 4);
  EXPECT_FLOAT_EQ(insets.top, 1);
  EXPECT_FLOAT_EQ(insets.right, 2);
  EXPECT_FLOAT_EQ(insets.bottom, 3);
  EXPECT_FLOAT_EQ(insets.left, 4);
}

TEST(EdgeInsetsTest, ApplyToRect) {
  EdgeInsets insets{10, 20, 30, 40};
  Rect r{0, 0, 200, 200};
  auto result = r.Inset(insets);
  EXPECT_FLOAT_EQ(result.x, 20);
  EXPECT_FLOAT_EQ(result.y, 10);
  EXPECT_FLOAT_EQ(result.width, 140);
  EXPECT_FLOAT_EQ(result.height, 160);
}

}  // namespace native::ui

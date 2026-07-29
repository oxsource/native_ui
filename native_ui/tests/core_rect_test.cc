#include "gtest/gtest.h"
#include "rect.h"
#include "point.h"

namespace native::ui {

TEST(RectTest, Contains) {
  Rect r{0, 0, 100, 100};
  EXPECT_TRUE(r.Contains({50, 50}));
  EXPECT_TRUE(r.Contains({0, 0}));
  EXPECT_TRUE(r.Contains({100, 100}));
  EXPECT_FALSE(r.Contains({-1, 50}));
  EXPECT_FALSE(r.Contains({101, 50}));
}

TEST(RectTest, Intersect_Overlapping) {
  Rect a{0, 0, 100, 100};
  Rect b{50, 50, 100, 100};
  auto result = a.Intersect(b);
  EXPECT_EQ(result.x, 50);
  EXPECT_EQ(result.y, 50);
  EXPECT_EQ(result.width, 50);
  EXPECT_EQ(result.height, 50);
}

TEST(RectTest, Intersect_NonOverlapping) {
  Rect a{0, 0, 10, 10};
  Rect b{20, 20, 10, 10};
  auto result = a.Intersect(b);
  EXPECT_EQ(result.width, 0);
  EXPECT_EQ(result.height, 0);
}

TEST(RectTest, Union) {
  Rect a{0, 0, 50, 50};
  Rect b{25, 25, 75, 75};
  auto result = a.Union(b);
  EXPECT_EQ(result.x, 0);
  EXPECT_EQ(result.y, 0);
  EXPECT_EQ(result.width, 100);
  EXPECT_EQ(result.height, 100);
}

TEST(RectTest, Offset) {
  Rect r{10, 20, 100, 50};
  auto result = r.Offset({5, -5});
  EXPECT_EQ(result.x, 15);
  EXPECT_EQ(result.y, 15);
  EXPECT_EQ(result.width, 100);
  EXPECT_EQ(result.height, 50);
}

}  // namespace native::ui

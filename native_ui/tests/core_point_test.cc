#include "gtest/gtest.h"
#include "src/framework/core/point.h"

namespace native::ui {

TEST(PointTest, Addition) {
  Point a{10, 20};
  Point b{30, 40};
  auto result = a + b;
  EXPECT_FLOAT_EQ(result.x, 40);
  EXPECT_FLOAT_EQ(result.y, 60);
}

TEST(PointTest, Subtraction) {
  Point a{50, 60};
  Point b{10, 20};
  auto result = a - b;
  EXPECT_FLOAT_EQ(result.x, 40);
  EXPECT_FLOAT_EQ(result.y, 40);
}

TEST(PointTest, DistanceTo) {
  Point a{0, 0};
  Point b{3, 4};
  EXPECT_FLOAT_EQ(a.DistanceTo(b), 5.0f);
}

TEST(PointTest, ZeroDistance) {
  Point a{10, 20};
  EXPECT_FLOAT_EQ(a.DistanceTo(a), 0.0f);
}

}  // namespace native::ui

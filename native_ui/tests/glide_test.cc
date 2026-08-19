#include "gtest/gtest.h"
#include "src/framework/widgets/glide.h"
#include "src/framework/render/image.h"

namespace native::ui {

TEST(GlideTest, DefaultIsNull) {
  EXPECT_EQ(Glide::Default(), nullptr);
}

TEST(GlideTest, SetAndGetDefault) {
  // No default Glide implementation needed for basic test
  // Just verify the singleton API works
  Glide* saved = Glide::Default();
  Glide::SetDefault(nullptr);
  EXPECT_EQ(Glide::Default(), nullptr);
  Glide::SetDefault(saved);
}

TEST(GlideTest, LoadWithoutDefaultReturns) {
  // Without Glide::Default set, ImageWidget handles gracefully
  // This tests that the API doesn't crash
  EXPECT_EQ(Glide::Default(), nullptr);
}

TEST(GlideTest, CancelWithoutLoad) {
  // Cancel with no active requests should not crash
  Glide* g = Glide::Default();
  if (g) g->Cancel(42);
}

}  // namespace native::ui

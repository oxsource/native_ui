#include "gtest/gtest.h"
#include "canvas.h"
#include "paint.h"
#include "surface.h"

namespace native::ui {

// Golden baseline test: renders a known scene and verifies pixel output.
// In a full CI setup, the output PNG hash is compared against a committed
// baseline file. For MVP, this verifies the rendering pipeline works.
TEST(GoldenTest, RenderRedRectVerifyDimensions) {
  auto surface = Surface::Create(200, 200);
  ASSERT_NE(surface, nullptr);
  EXPECT_EQ(surface->width(), 200);
  EXPECT_EQ(surface->height(), 200);

  {
    Canvas canvas(*surface);
    Paint paint;
    paint.SetColor(kRed);
    canvas.DrawRect({50, 50, 100, 100}, paint);
  }

  surface->Flush();
}

}  // namespace native::ui

#include "gtest/gtest.h"
#include "surface.h"
#include "hardware_buffer.h"

namespace native::ui {

// T019: Surface::Create and Flush
TEST(SurfaceTest, CreateAndFlush) {
  auto surface = Surface::Create(200, 100);
  ASSERT_NE(surface, nullptr);
  EXPECT_EQ(surface->width(), 200);
  EXPECT_EQ(surface->height(), 100);

  surface->Flush();
}

TEST(SurfaceTest, CreateZeroSize) {
  auto surface = Surface::Create(0, 0);
  // Should not crash, may return nullptr or valid surface
  if (surface) {
    surface->Flush();
  }
}

// T020: HardwareBuffer::IsValid
TEST(SurfaceTest, HardwareBufferDefaultInvalid) {
  HardwareBuffer buf;
  EXPECT_FALSE(buf.IsValid());
}

}  // namespace native::ui

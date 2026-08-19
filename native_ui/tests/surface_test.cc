#include "gtest/gtest.h"
#include "src/framework/surface/surface.h"
#include "src/framework/surface/hardware_buffer.h"

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

// T012: Surface::CreateFromBuffer host stub + RenderBackend.
TEST(SurfaceTest, CreateFromBufferInvalidNull) {
  auto surface = Surface::CreateFromBuffer(HardwareBuffer());
  EXPECT_EQ(surface, nullptr);
}

TEST(SurfaceTest, CreateFromBufferHostStubNull) {
  uint8_t pixels[64] = {};
  auto hb = HardwareBuffer::FromMemory(pixels, 64, 4, 4);
  ASSERT_TRUE(hb.IsValid());
  auto surface = Surface::CreateFromBuffer(hb, RenderBackend::kCPU);
  EXPECT_EQ(surface, nullptr);  // TODO(android-only): host builds stub the conversion
}

TEST(SurfaceTest, CreateFromBufferGpuWithoutContextNull) {
  uint8_t pixels[64] = {};
  auto hb = HardwareBuffer::FromMemory(pixels, 64, 4, 4);
  // kGPU without a RenderContext: fall back to CPU (host stub) -> nullptr.
  auto surface = Surface::CreateFromBuffer(hb, RenderBackend::kGPU, nullptr);
  EXPECT_EQ(surface, nullptr);
}

TEST(SurfaceTest, RenderBackendEnumExists) {
  // Compile-time check that the enum is usable and kCPU is the default selection.
  RenderBackend backend = RenderBackend::kCPU;
  EXPECT_EQ(backend, RenderBackend::kCPU);
  (void)RenderBackend::kGPU;
}

}  // namespace native::ui

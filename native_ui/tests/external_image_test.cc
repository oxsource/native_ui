#include "gtest/gtest.h"

#include "canvas.h"
#include "external_image.h"
#include "hardware_buffer.h"
#include "image.h"
#include "surface.h"

namespace native::ui {

// ============================================================================
// T010: Host contract tests for the HardwareBuffer wrapper and ExternalImage.
// Android is the only implemented platform; on host the conversion APIs are
// guarded stubs (// TODO(android-only)), so these tests pin the wrapper contract,
// the stub behavior, and the no-crash guarantees (FR-005/FR-012).
// ============================================================================

TEST(ExternalImageTest, FromMemoryGeometry) {
  uint8_t pixels[4 * 4 * 4] = {};
  auto hb = HardwareBuffer::FromMemory(pixels, 16 * 4, 16, 4);
  ASSERT_TRUE(hb.IsValid());
  EXPECT_EQ(hb.kind(), HardwareBuffer::Kind::kMemory);
  EXPECT_EQ(hb.width(), 16);
  EXPECT_EQ(hb.height(), 4);
  EXPECT_EQ(hb.row_bytes(), static_cast<size_t>(16 * 4));
  EXPECT_EQ(hb.pixels(), pixels);
}

TEST(ExternalImageTest, FromMemoryInvalid) {
  EXPECT_FALSE(HardwareBuffer::FromMemory(nullptr, 16, 0, 0).IsValid());
  EXPECT_FALSE(HardwareBuffer::FromMemory(nullptr, 16, 4, 4).IsValid());
}

// operator== compares the underlying handle (used to skip redundant conversion).
TEST(ExternalImageTest, EqualityComparesHandle) {
  uint8_t pixels[64] = {};
  auto a = HardwareBuffer::FromMemory(pixels, 64, 4, 4);
  auto b = HardwareBuffer::FromMemory(pixels, 64, 4, 4);
  auto c = HardwareBuffer::FromMemory(pixels, 64, 8, 8);  // same handle, other dims
  EXPECT_TRUE(a == b);
  EXPECT_TRUE(a == c);
  EXPECT_FALSE(a == HardwareBuffer());  // invalid buffer has null handle
}

TEST(ExternalImageTest, DefaultInvalid) {
  HardwareBuffer hb;
  EXPECT_FALSE(hb.IsValid());
  EXPECT_EQ(hb.kind(), HardwareBuffer::Kind::kInvalid);
}

TEST(ExternalImageTest, DrawWithoutBufferNoCrash) {
  ExternalImage ext;  // bound to nothing (invalid buffer)
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    ext.Draw(canvas);
  }
}

TEST(ExternalImageTest, SetInvalidBufferNoCrash) {
  ExternalImage ext;
  ext.SetBuffer(HardwareBuffer());
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    ext.Draw(canvas);
  }
}

TEST(ExternalImageTest, SetMemoryBufferHostStubNoCrash) {
  uint8_t pixels[64] = {};
  ExternalImage ext;
  ext.SetBuffer(HardwareBuffer::FromMemory(pixels, 64, 4, 4));
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  {
    Canvas canvas(*surface);
    ext.Draw(canvas);  // image_ stays null on host (stub); Draw no-ops
  }
}

// Host stub contract: Image::FromBuffer returns nullptr for any buffer.
TEST(ExternalImageTest, FromBufferHostStubReturnsNull) {
  uint8_t pixels[64] = {};
  auto hb = HardwareBuffer::FromMemory(pixels, 64, 4, 4);
  auto img = Image::FromBuffer(hb);
  EXPECT_EQ(img, nullptr);  // TODO(android-only): host builds stub the conversion
}

}  // namespace native::ui

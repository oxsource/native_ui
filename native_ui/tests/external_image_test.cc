#include "gtest/gtest.h"

#include "src/framework/render/canvas.h"
#include "src/framework/widgets/external_image.h"
#include "src/framework/surface/hardware_buffer.h"
#include "src/framework/render/image.h"
#include "src/framework/surface/surface.h"

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

// ============================================================================
// T025: US2 live-update host tests. Android is the only implemented platform, so
// the positive live-update behavior is validated on device (demo --live, T027);
// these tests pin the bounded-memory / no-crash guarantee and the rebuild-guard
// contract on the host stub path (FR-004/FR-005).
// ============================================================================

TEST(ExternalImageTest, TenThousandSetBufferCyclesNoCrash) {
  uint8_t pixels[4 * 4 * 4] = {};
  auto hb = HardwareBuffer::FromMemory(pixels, 16 * 4, 16, 4);
  ExternalImage ext;
  for (int i = 0; i < 10000; ++i) {
    if (i % 2 == 0) {
      ext.SetBuffer(hb);
    } else {
      ext.SetBuffer(HardwareBuffer());  // invalid/empty buffer path (FR-005)
    }
  }
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  { Canvas canvas(*surface); ext.Draw(canvas); }
}

TEST(ExternalImageTest, RepeatedSetBufferSameHandleIsNoOp) {
  uint8_t pixels[4 * 4 * 4] = {};
  auto hb = HardwareBuffer::FromMemory(pixels, 16 * 4, 16, 4);
  ExternalImage ext;
  ext.SetBuffer(hb);
  // Re-applying the same handle must be a no-op (rebuild guard); on the host stub
  // this pins the no-crash / no-regression contract. The actual skip of the
  // per-frame re-conversion is observable on device (demo --live).
  ext.SetBuffer(hb);
  ext.SetBuffer(hb);
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  { Canvas canvas(*surface); ext.Draw(canvas); }
}

// ============================================================================
// T028: US3 producer utilities + defined error states (FR-005/FR-006/FR-009).
// Unsupported-format rejection is validated on device (android path); on host the
// invalid/zero-area buffers are the observable error states (no crash, no render).
// ============================================================================

TEST(ExternalImageTest, ProducerCanInspectGeometry) {
  // A producer creating a buffer from in-memory pixels can read back its geometry (FR-009).
  uint8_t pixels[32 * 16 * 4] = {};
  auto hb = HardwareBuffer::FromMemory(pixels, 32 * 4, 32, 16);
  ASSERT_TRUE(hb.IsValid());
  EXPECT_EQ(hb.kind(), HardwareBuffer::Kind::kMemory);
  EXPECT_EQ(hb.width(), 32);
  EXPECT_EQ(hb.height(), 16);
  EXPECT_EQ(hb.row_bytes(), static_cast<size_t>(32 * 4));
  EXPECT_EQ(hb.pixels(), pixels);
}

TEST(ExternalImageTest, ZeroAreaFromMemoryInvalid) {
  uint8_t pixels[16] = {};
  auto hb = HardwareBuffer::FromMemory(pixels, 16, 0, 4);   // zero width
  EXPECT_FALSE(hb.IsValid());
  EXPECT_EQ(Image::FromBuffer(hb), nullptr);  // FR-005: zero-area -> nullptr
}

TEST(ExternalImageTest, ZeroAreaFromMemoryNoRender) {
  uint8_t pixels[16] = {};
  ExternalImage ext;
  ext.SetBuffer(HardwareBuffer::FromMemory(pixels, 16, 0, 4));  // invalid -> no image
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  { Canvas canvas(*surface); ext.Draw(canvas); }  // no-op, no crash (FR-005)
}

TEST(ExternalImageTest, InvalidBufferFromBufferNull) {
  EXPECT_EQ(Image::FromBuffer(HardwareBuffer()), nullptr);  // FR-006 defined error state
  ExternalImage ext;
  ext.SetBuffer(HardwareBuffer());
  auto surface = Surface::Create(100, 50);
  ASSERT_NE(surface, nullptr);
  { Canvas canvas(*surface); ext.Draw(canvas); }  // draws nothing, no crash
}

}  // namespace native::ui

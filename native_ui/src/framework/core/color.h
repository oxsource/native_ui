#pragma once

#include <cstdint>
#include <algorithm>

namespace native::ui {

// Framework-owned color space (mapped to the renderer internally). Keeps the
// rendering API decoupled from any Skia color-space types.
enum class ColorSpace { kSRGB, kLinearSRGB };

struct Color {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t a = 255;

  constexpr Color() = default;

  constexpr Color(int red, int green, int blue, int alpha = 255)
      : r(Clamp(red)), g(Clamp(green)), b(Clamp(blue)), a(Clamp(alpha)) {}

  static constexpr uint8_t Clamp(int val) {
    return static_cast<uint8_t>(val < 0 ? 0 : val > 255 ? 255 : val);
  }
};

constexpr Color kRed{255, 0, 0};
constexpr Color kGreen{0, 255, 0};
constexpr Color kBlue{0, 0, 255};
constexpr Color kWhite{255, 255, 255};
constexpr Color kBlack{0, 0, 0};
constexpr Color kTransparent{0, 0, 0, 0};

}  // namespace native::ui

#pragma once

#include <cstdint>
#include <algorithm>

namespace native::ui {

struct Color {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t a = 255;

  constexpr Color() = default;

  constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
      : r(red), g(green), b(blue), a(alpha) {}

  constexpr explicit Color(int red, int green, int blue, int alpha = 255)
      : r(Clamp(red)), g(Clamp(green)), b(Clamp(blue)), a(Clamp(alpha)) {}

private:
  static constexpr uint8_t Clamp(int val) {
    return static_cast<uint8_t>(val < 0 ? 0 : val > 255 ? 255 : val);
  }
};

constexpr Color kRed{uint8_t{255}, uint8_t{0}, uint8_t{0}, uint8_t{255}};
constexpr Color kGreen{uint8_t{0}, uint8_t{255}, uint8_t{0}, uint8_t{255}};
constexpr Color kBlue{uint8_t{0}, uint8_t{0}, uint8_t{255}, uint8_t{255}};
constexpr Color kWhite{uint8_t{255}, uint8_t{255}, uint8_t{255}, uint8_t{255}};
constexpr Color kBlack{uint8_t{0}, uint8_t{0}, uint8_t{0}, uint8_t{255}};
constexpr Color kTransparent{uint8_t{0}, uint8_t{0}, uint8_t{0}, uint8_t{0}};

}  // namespace native::ui

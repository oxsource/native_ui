#pragma once

#include "point.h"

namespace native::ui {

struct MouseButton {
  int value;
  static constexpr int kLeft = 0;
  static constexpr int kRight = 1;
  static constexpr int kMiddle = 2;
};

struct ModifierFlags {
  int value;
  static constexpr int kNone  = 0;
  static constexpr int kShift = 1 << 0;
  static constexpr int kCtrl  = 1 << 1;
  static constexpr int kAlt   = 1 << 2;
  static constexpr int kMeta  = 1 << 3;
};

struct MouseEvent {
  Point position;
  int button = 0;
  int modifiers = 0;
};

struct KeyEvent {
  int key_code = 0;
  int modifiers = 0;
};

}  // namespace native::ui

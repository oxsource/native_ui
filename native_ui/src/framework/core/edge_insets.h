#pragma once

namespace native::ui {

struct EdgeInsets {
  float top = 0;
  float left = 0;
  float bottom = 0;
  float right = 0;

  static EdgeInsets All(float v);
  static EdgeInsets Symmetric(float horizontal, float vertical);
  static EdgeInsets Only(float top, float right, float bottom, float left);
};

}  // namespace native::ui

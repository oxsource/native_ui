#include "src/framework/core/edge_insets.h"

namespace native::ui {

EdgeInsets EdgeInsets::All(float v) {
  return {v, v, v, v};
}

EdgeInsets EdgeInsets::Symmetric(float horizontal, float vertical) {
  return {vertical, horizontal, vertical, horizontal};
}

EdgeInsets EdgeInsets::Only(float t, float r, float b, float l) {
  return {t, l, b, r};
}

}  // namespace native::ui

#include "size.h"

namespace native::ui {

bool Size::IsEmpty() const {
  return width <= 0 || height <= 0;
}

bool operator==(const Size& a, const Size& b) {
  return a.width == b.width && a.height == b.height;
}

}  // namespace native::ui

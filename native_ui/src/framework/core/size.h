#pragma once

namespace native::ui {

struct Size {
  float width = 0;
  float height = 0;

  bool IsEmpty() const;
};

bool operator==(const Size& a, const Size& b);

}  // namespace native::ui

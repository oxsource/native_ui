#pragma once

#include "src/framework/widgets/widget.h"
#include "src/framework/core/point.h"

namespace native::ui {

struct HitTestResult {
  Widget* widget = nullptr;
  Point local_pos;
};

class HitTester {
public:
  HitTestResult Test(Widget* root, Point point);
};

}  // namespace native::ui

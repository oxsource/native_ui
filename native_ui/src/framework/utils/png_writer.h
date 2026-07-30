#pragma once

#include "SkSurface.h"

namespace native::ui {

class PngWriter {
public:
  static bool Write(SkSurface* surface, const char* path);
};

}  // namespace native::ui

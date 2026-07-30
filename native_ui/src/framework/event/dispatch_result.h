#pragma once

#include "widget.h"

namespace native::ui {

enum class DispatchStatus {
  kHandled,
  kUnhandled,
  kRejected,
  kNoTarget,
};

struct DispatchResult {
  DispatchStatus status = DispatchStatus::kNoTarget;
  Widget* target = nullptr;
};

}  // namespace native::ui

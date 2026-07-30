#include "event.h"

namespace native::ui {

DispatchResult EventHub::Push(const MouseEvent& event) {
  // Phase 1: Filter chain — any filter can reject
  for (const auto& filter : filters_) {
    if (!filter(event)) {
      return DispatchResult{DispatchStatus::kRejected, nullptr};
    }
  }

  if (!root_) return DispatchResult{DispatchStatus::kNoTarget, nullptr};

  // Phase 2: Hit test to find target widget
  HitTestResult hit = hit_tester_.Test(root_, event.position);
  if (!hit.widget) {
    return DispatchResult{DispatchStatus::kNoTarget, nullptr};
  }

  // Phase 3: Dispatch to target (direct dispatch for MVP, bubble deferred)
  return DispatchToTarget(event, hit);
}

DispatchResult EventHub::Push(const KeyEvent& event) {
  if (!root_) return DispatchResult{DispatchStatus::kNoTarget, nullptr};

  // Walk root and find first widget that handles the key event
  // For MVP: try root, then DFS children
  if (root_->OnKeyEvent(event)) {
    return DispatchResult{DispatchStatus::kHandled, root_};
  }
  for (int i = 0; i < root_->ChildCount(); ++i) {
    Widget* child = root_->ChildAt(i);
    if (child && child->OnKeyEvent(event)) {
      return DispatchResult{DispatchStatus::kHandled, child};
    }
  }
  return DispatchResult{DispatchStatus::kUnhandled, root_};
}

void EventHub::AddFilter(EventFilter filter) {
  filters_.push_back(std::move(filter));
}

DispatchResult EventHub::DispatchToTarget(const MouseEvent& event, HitTestResult hit) {
  Widget* target = hit.widget;

  // Call OnMouseEvent on target widget
  if (target->OnMouseEvent(event)) {
    return DispatchResult{DispatchStatus::kHandled, target};
  }

  // Bubble phase deferred: without parent pointers, direct dispatch only
  return DispatchResult{DispatchStatus::kUnhandled, target};
}

}  // namespace native::ui

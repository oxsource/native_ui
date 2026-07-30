#pragma once

#include <functional>
#include <vector>

#include "dispatch_result.h"
#include "event_types.h"
#include "hit_tester.h"

namespace native::ui {

using EventFilter = std::function<bool(const MouseEvent&)>;

class EventHub {
public:
  void SetRoot(Widget* root) { root_ = root; }
  Widget* root() const { return root_; }

  DispatchResult Push(const MouseEvent& event);
  DispatchResult Push(const KeyEvent& event);

  void AddFilter(EventFilter filter);
  HitTester& hit_tester() { return hit_tester_; }

private:
  DispatchResult DispatchToTarget(const MouseEvent& event, HitTestResult hit);

  Widget* root_ = nullptr;
  std::vector<EventFilter> filters_;
  HitTester hit_tester_;
};

}  // namespace native::ui

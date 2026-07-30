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
  DispatchResult Push(const MouseEvent& event);
  DispatchResult Push(const KeyEvent& event);

  void AddFilter(EventFilter filter);
  HitTester& hit_tester() { return hit_tester_; }

private:
  std::vector<EventFilter> filters_;
  HitTester hit_tester_;
};

}  // namespace native::ui

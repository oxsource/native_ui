#include "src/framework/widgets/widget.h"
#include "src/framework/widgets/event_types.h"

namespace native::ui {

Widget* Widget::FindById(const std::string& id) {
  if (id_ == id) return this;
  for (int i = 0; i < ChildCount(); ++i) {
    auto* found = ChildAt(i)->FindById(id);
    if (found) return found;
  }
  return nullptr;
}

void Widget::RequestLayout() {
  needs_layout_ = true;
  needs_draw_ = true;
}

void Widget::RequestRedraw() {
  needs_draw_ = true;
}

void Widget::UnwatchAll() {
  if (watched_state_) {
    watched_state_->RemoveWatcher(this);
    watched_state_ = nullptr;
  }
}

bool Widget::OnMouseEvent(const MouseEvent&) { return false; }
bool Widget::OnKeyEvent(const KeyEvent&) { return false; }

}  // namespace native::ui

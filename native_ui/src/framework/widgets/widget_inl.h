#ifndef NATIVE_UI_WIDGET_INL_H
#define NATIVE_UI_WIDGET_INL_H

#include "state.h"

namespace native::ui {

template<typename T>
void Widget::Watch(Property<T>& prop) {
  State* s = prop.state();
  if (s) {
    s->AddWatcher(this, [this]() { this->RequestRedraw(); }, &prop);
    watched_state_ = s;
  }
}

}  // namespace native::ui

#endif  // NATIVE_UI_WIDGET_INL_H

// This file is included at the bottom of state.h after State is fully defined.
// It provides the implementation of Property<T>::Signal() which depends on State.

#ifndef NATIVE_UI_PROPERTY_INL_H
#define NATIVE_UI_PROPERTY_INL_H

namespace native::ui {

template<typename T>
void Property<T>::Signal() {
  if (owner_) {
    owner_->EnqueueDirty(this);
  }
}

}  // namespace native::ui

#endif  // NATIVE_UI_PROPERTY_INL_H

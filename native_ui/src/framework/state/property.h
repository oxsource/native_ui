#pragma once

#include <functional>

#include "src/framework/state/property_base.h"

namespace native::ui {

class State;

template<typename T>
class Property : public PropertyBase {
public:
  explicit Property(State* owner)
      : owner_(owner) {}

  Property& operator=(const T& val) {
    if (before_set_) before_set_(val);
    value_ = val;
    Signal();
    if (after_set_) after_set_(val);
    return *this;
  }

  const T& value() const { return value_; }
  operator const T&() const { return value_; }

  void OnBeforeSet(std::function<void(const T&)> fn) {
    before_set_ = std::move(fn);
  }
  void OnAfterSet(std::function<void(const T&)> fn) {
    after_set_ = std::move(fn);
  }

  State* state() const override { return owner_; }

  // Implementation is in property_inl.h (included by state.h)
  void Signal() override;

private:
  State* owner_;
  T value_{};
  std::function<void(const T&)> before_set_;
  std::function<void(const T&)> after_set_;
};

}  // namespace native::ui

#pragma once

namespace native::ui {

class State;

class PropertyBase {
public:
  virtual ~PropertyBase() = default;

  virtual void Signal() = 0;
  virtual State* state() const = 0;

  PropertyBase* key() const { return const_cast<PropertyBase*>(this); }
};

}  // namespace native::ui

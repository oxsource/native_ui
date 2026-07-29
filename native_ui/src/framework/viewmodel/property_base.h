#pragma once

namespace native::ui {

class PropertyBase {
public:
  virtual ~PropertyBase() = default;

  virtual void Signal() = 0;

  PropertyBase* key() const { return const_cast<PropertyBase*>(this); }
};

}  // namespace native::ui

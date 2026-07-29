#pragma once

#include <string>
#include <vector>

#include "property.h"

namespace native::ui {

class Widget {
public:
  virtual ~Widget() = default;

  void SetId(std::string id) { id_ = std::move(id); }
  const std::string& GetId() const { return id_; }

  virtual Widget* ChildAt(int index) { return nullptr; }
  virtual int ChildCount() const { return 0; }
  virtual int IndexOf(Widget* child) const { return -1; }

  Widget* FindById(const std::string& id);

  void RequestLayout();
  virtual void RequestRedraw();

  template<typename T>
  void Watch(Property<T>& prop);

  void UnwatchAll();

  virtual void OnMount() {}
  virtual void OnUnmount() {}
  virtual void Draw(class Canvas&) = 0;

  bool needs_layout() const { return needs_layout_; }
  bool needs_draw() const { return needs_draw_; }

private:
  std::string id_;
  bool needs_layout_ = false;
  bool needs_draw_ = false;
  State* watched_state_ = nullptr;
};

}  // namespace native::ui

#include "widget_inl.h"

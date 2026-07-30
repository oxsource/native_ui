#pragma once

#include <memory>
#include <vector>

#include "widget.h"

namespace native::ui {

class Stack : public Widget {
public:
  template <typename... Args>
  explicit Stack(Args&&... args) {
    (ProcessArg(std::forward<Args>(args)), ...);
  }

  struct Children {
    std::vector<std::unique_ptr<Widget>> value;
  };

  void AddChild(std::unique_ptr<Widget> child);
  void RemoveChild(Widget* child);
  void ClearChildren();

  Widget* ChildAt(int index) override;
  int ChildCount() const override;
  int IndexOf(Widget* child) const override;
  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(Children tag);
  void ProcessArg(Id tag);

  std::vector<std::unique_ptr<Widget>> children_;
};

}  // namespace native::ui

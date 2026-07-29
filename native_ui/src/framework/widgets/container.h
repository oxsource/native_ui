#pragma once

#include <memory>
#include <vector>

#include "widget.h"
#include "yoga/Yoga.h"

namespace native::ui {

struct Direction {
  int value;
  static constexpr int kRow = 0;
  static constexpr int kColumn = 1;
};

struct Padding {
  float value;
};

struct Gap {
  float value;
};

struct Margin {
  float value;
};

struct Id {
  std::string value;
};

class Container : public Widget {
public:
  template <typename... Args>
  explicit Container(Args&&... args) {
    root_ = YGNodeNew();
    (ProcessArg(std::forward<Args>(args)), ...);
  }

  struct Children {
    std::vector<std::unique_ptr<Widget>> value;
  };

  ~Container() override;

  void AddChild(std::unique_ptr<Widget> child);
  void RemoveChild(Widget* child);
  void ClearChildren();

  Widget* ChildAt(int index) override;
  int ChildCount() const override;
  int IndexOf(Widget* child) const override;

  void Draw(class Canvas& canvas) override;

private:
  void ProcessArg(Direction tag);
  void ProcessArg(Padding tag);
  void ProcessArg(Gap tag);
  void ProcessArg(Margin tag);
  void ProcessArg(Children tag);
  void ProcessArg(Id tag);

  std::vector<std::unique_ptr<Widget>> children_;
  std::vector<YGNodeRef> child_nodes_;
  YGNodeRef root_ = nullptr;
};

}  // namespace native::ui

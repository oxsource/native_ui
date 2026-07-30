#pragma once

#include <memory>
#include <vector>

#include "widget.h"
#include "flex_layout.h"
#include "layout_result.h"

namespace native::ui {

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

  void Measure(Size available);
  void Arrange(Size container_size);
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
  std::unique_ptr<FlexLayout> layout_;
  std::vector<MeasureResult> layout_result_;
};

}  // namespace native::ui

#pragma once

#include <memory>
#include <vector>

#include "src/framework/widgets/widget.h"
#include "src/framework/layout/flex_layout.h"
#include "src/framework/layout/layout_result.h"

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

  // Bring Widget base ProcessArg overloads into scope (prevent name hiding)
  using Widget::ProcessArg;

  ~Container() override;

  void AddChild(std::unique_ptr<Widget> child);
  void RemoveChild(Widget* child);
  void ClearChildren();

  Widget* ChildAt(int index) override;
  int ChildCount() const override;
  int IndexOf(Widget* child) const override;

  void Layout();
  void Layout(float w, float h) { style_.setWidth(w); style_.setHeight(h); Layout(); }
  void Measure();
  void Arrange();
  void Draw(class Canvas& canvas) override;

private:
  void ProcessArg(Direction tag);
  void ProcessArg(JustifyContent tag);
  void ProcessArg(AlignItems tag);
  void ProcessArg(Gap tag);
  void ProcessArg(Margin tag);
  void ProcessArg(Children tag);
  void ProcessArg(Id tag);

  void PrepareLayout();
  void ReadChildLayout();
  void PropagateLayout();

  std::vector<std::unique_ptr<Widget>> children_;
  std::vector<YGNodeRef> child_nodes_;
  YGNodeRef root_ = nullptr;
  std::unique_ptr<FlexLayout> layout_;
  std::vector<MeasureResult> layout_result_;
};

}  // namespace native::ui

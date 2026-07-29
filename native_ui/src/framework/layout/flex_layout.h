#pragma once

#include <vector>

#include "layout_result.h"
#include "yoga/Yoga.h"

namespace native::ui {

// Tag types
struct Direction { int value; };
struct JustifyContent { int value; };
struct AlignItems { int value; };
struct AlignContent { int value; };
struct FlexWrap { int value; };
struct Gap { float value; };
struct Padding { float value; };
struct Margin { float value; };

class FlexLayout {
public:
  template <typename... Args>
  explicit FlexLayout(Args&&... args) {
    (ProcessArg(std::forward<Args>(args)), ...);
  }

  ~FlexLayout();

  void SetChildren(const std::vector<YGNodeRef>& children);
  std::vector<MeasureResult> Measure(Size available_size);
  void Arrange(std::vector<MeasureResult>& measured, Size container_size);

private:
  void ProcessArg(Direction tag);
  void ProcessArg(JustifyContent tag);
  void ProcessArg(AlignItems tag);
  void ProcessArg(AlignContent tag);
  void ProcessArg(FlexWrap tag);
  void ProcessArg(Gap tag);
  void ProcessArg(Padding tag);
  void ProcessArg(Margin tag);

  YGNodeRef root_ = nullptr;
  std::vector<YGNodeRef> children_;
};

}  // namespace native::ui

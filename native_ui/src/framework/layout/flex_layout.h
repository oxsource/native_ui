#pragma once

#include <vector>

#include "layout_result.h"
#include "yoga/Yoga.h"

namespace native::ui {

// Tag types with constants
struct Direction {
  int value;
  static constexpr int kRow = 0;
  static constexpr int kColumn = 1;
};
struct JustifyContent {
  int value;
  static constexpr int kFlexStart = 0;
  static constexpr int kCenter = 1;
  static constexpr int kFlexEnd = 2;
  static constexpr int kSpaceBetween = 3;
  static constexpr int kSpaceAround = 4;
  static constexpr int kSpaceEvenly = 5;
};
struct AlignItems {
  int value;
  static constexpr int kAuto = 0;
  static constexpr int kFlexStart = 1;
  static constexpr int kCenter = 2;
  static constexpr int kFlexEnd = 3;
  static constexpr int kStretch = 4;
  static constexpr int kBaseline = 5;
  static constexpr int kSpaceBetween = 6;
  static constexpr int kSpaceAround = 7;
};
struct AlignContent {
  int value;
  static constexpr int kAuto = 0;
  static constexpr int kFlexStart = 1;
  static constexpr int kCenter = 2;
  static constexpr int kFlexEnd = 3;
  static constexpr int kStretch = 4;
  static constexpr int kBaseline = 5;
  static constexpr int kSpaceBetween = 6;
  static constexpr int kSpaceAround = 7;
};
struct FlexWrap {
  int value;
  static constexpr int kNoWrap = 0;
  static constexpr int kWrap = 1;
  static constexpr int kWrapReverse = 2;
};
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

  YGNodeRef root_ = YGNodeNew();
  std::vector<YGNodeRef> children_;
};

}  // namespace native::ui

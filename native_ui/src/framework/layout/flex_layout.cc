#include "src/framework/layout/flex_layout.h"

namespace native::ui {

void FlexLayout::ProcessArg(Direction tag) {
  YGNodeStyleSetFlexDirection(
      root_, tag.value == 0 ? YGFlexDirectionRow : YGFlexDirectionColumn);
}

void FlexLayout::ProcessArg(JustifyContent tag) {
  YGNodeStyleSetJustifyContent(root_, static_cast<YGJustify>(tag.value));
}

void FlexLayout::ProcessArg(AlignItems tag) {
  YGNodeStyleSetAlignItems(root_, static_cast<YGAlign>(tag.value));
}

void FlexLayout::ProcessArg(AlignContent tag) {
  YGNodeStyleSetAlignContent(root_, static_cast<YGAlign>(tag.value));
}

void FlexLayout::ProcessArg(FlexWrap tag) {
  YGNodeStyleSetFlexWrap(root_, static_cast<YGWrap>(tag.value));
}

void FlexLayout::ProcessArg(Gap tag) {
  YGNodeStyleSetGap(root_, YGGutterAll, tag.value);
}

void FlexLayout::ProcessArg(Margin tag) {
  YGNodeStyleSetMargin(root_, YGEdgeAll, tag.value);
}

FlexLayout::~FlexLayout() {
  YGNodeFreeRecursive(root_);
}

void FlexLayout::SetChildren(const std::vector<YGNodeRef>& children) {
  children_ = children;
}

std::vector<MeasureResult> FlexLayout::Measure(Size available) {
  YGNodeStyleSetWidth(root_, available.width);
  YGNodeStyleSetHeight(root_, available.height);

  for (size_t i = 0; i < children_.size(); i++)
    YGNodeInsertChild(root_, children_[i], static_cast<int32_t>(i));

  YGNodeCalculateLayout(root_, YGUndefined, YGUndefined, YGDirectionLTR);

  std::vector<MeasureResult> results;
  for (auto* child : children_) {
    results.push_back({
      Size{YGNodeLayoutGetWidth(child), YGNodeLayoutGetHeight(child)},
      Point{0, 0}
    });
  }
  return results;
}

void FlexLayout::Arrange(std::vector<MeasureResult>& measured, Size) {
  for (size_t i = 0; i < children_.size(); i++) {
    measured[i].position = Point{
      YGNodeLayoutGetLeft(children_[i]),
      YGNodeLayoutGetTop(children_[i])
    };
  }
}

}  // namespace native::ui

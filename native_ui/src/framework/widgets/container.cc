#include "src/framework/widgets/container.h"
#include "src/framework/render/canvas.h"
#include "src/framework/render/paint.h"

namespace native::ui {

Container::~Container() = default;

void Container::ProcessArg(Direction tag) {
  YGNodeStyleSetFlexDirection(
      root_, static_cast<YGFlexDirection>(tag.value));
}

void Container::ProcessArg(JustifyContent tag) {
  YGNodeStyleSetJustifyContent(root_, static_cast<YGJustify>(tag.value));
}

void Container::ProcessArg(AlignItems tag) {
  YGNodeStyleSetAlignItems(root_, static_cast<YGAlign>(tag.value));
}

void Container::ProcessArg(Gap tag) {
  YGNodeStyleSetGap(root_, YGGutterAll, tag.value);
}

void Container::ProcessArg(Margin tag) {
  YGNodeStyleSetMargin(root_, YGEdgeAll, tag.value);
}

void Container::ProcessArg(Children tag) {
  for (auto& child : tag.value) {
    AddChild(std::move(child));
  }
}

void Container::ProcessArg(Id tag) {
  SetId(std::move(tag.value));
}

void Container::AddChild(std::unique_ptr<Widget> child) {
  auto* container = dynamic_cast<Container*>(child.get());
  YGNodeRef child_node = container ? container->root_ : YGNodeNew();
  YGNodeStyleSetFlexGrow(child_node, 1);
  YGNodeStyleSetFlexShrink(child_node, 1);
  YGNodeInsertChild(root_, child_node, static_cast<int32_t>(child_nodes_.size()));
  child_nodes_.push_back(child_node);
  children_.push_back(std::move(child));
  children_.back()->OnMount();
  RequestLayout();
}

void Container::RemoveChild(Widget* child) {
  for (size_t i = 0; i < children_.size(); ++i) {
    if (children_[i].get() == child) {
      YGNodeRemoveChild(root_, child_nodes_[i]);
      if (!dynamic_cast<Container*>(child)) YGNodeFree(child_nodes_[i]);
      child_nodes_.erase(child_nodes_.begin() + static_cast<ptrdiff_t>(i));
      children_.erase(children_.begin() + static_cast<ptrdiff_t>(i));
      RequestLayout();
      return;
    }
  }
}

void Container::ClearChildren() {
  for (size_t i = 0; i < child_nodes_.size(); ++i) {
    YGNodeRemoveChild(root_, child_nodes_[i]);
    if (!dynamic_cast<Container*>(children_[i].get())) YGNodeFree(child_nodes_[i]);
  }
  child_nodes_.clear();
  children_.clear();
  RequestLayout();
}

Widget* Container::ChildAt(int index) {
  if (index < 0 || static_cast<size_t>(index) >= children_.size()) {
    return nullptr;
  }
  return children_[static_cast<size_t>(index)].get();
}

int Container::ChildCount() const {
  return static_cast<int>(children_.size());
}

int Container::IndexOf(Widget* child) const {
  for (size_t i = 0; i < children_.size(); ++i) {
    if (children_[i].get() == child) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void Container::Layout() {
  PrepareLayout();
  Measure();
  Arrange();
  PropagateLayout();
}

void Container::PrepareLayout() {
  for (size_t i = 0; i < children_.size(); i++) {
    float cw = children_[i]->style().width();
    float ch = children_[i]->style().height();
    if (cw > 0) YGNodeStyleSetWidth(child_nodes_[i], cw);
    if (ch > 0) {
      YGNodeStyleSetHeight(child_nodes_[i], ch);
      YGNodeStyleSetFlexGrow(child_nodes_[i], 0);
    }
    if (auto* c = dynamic_cast<Container*>(children_[i].get())) {
      c->PrepareLayout();
    }
  }
}

void Container::PropagateLayout() {
  for (size_t i = 0; i < children_.size(); ++i) {
    if (auto* c = dynamic_cast<Container*>(children_[i].get())) {
      c->ReadChildLayout();
      c->Arrange();
      c->PropagateLayout();
    }
  }
}

void Container::Measure() {
  float w = style().width() > 0 ? style().width() : YGUndefined;
  float h = style().height() > 0 ? style().height() : YGUndefined;
  YGNodeStyleSetWidth(root_, w);
  YGNodeStyleSetHeight(root_, h);

  // Apply padding from Style
  auto pad = style().padding();
  YGNodeStyleSetPadding(root_, YGEdgeLeft, pad.left);
  YGNodeStyleSetPadding(root_, YGEdgeTop, pad.top);
  YGNodeStyleSetPadding(root_, YGEdgeRight, pad.right);
  YGNodeStyleSetPadding(root_, YGEdgeBottom, pad.bottom);

  YGNodeCalculateLayout(root_, YGUndefined, YGUndefined, YGDirectionLTR);

  ReadChildLayout();
}

void Container::ReadChildLayout() {
  layout_result_.resize(child_nodes_.size());
  for (size_t i = 0; i < child_nodes_.size(); i++) {
    layout_result_[i].size = Size{
      YGNodeLayoutGetWidth(child_nodes_[i]),
      YGNodeLayoutGetHeight(child_nodes_[i])};
    layout_result_[i].position = Point{0, 0};
  }
}

void Container::Arrange() {
  for (size_t i = 0; i < child_nodes_.size(); i++) {
    layout_result_[i].position = Point{
        YGNodeLayoutGetLeft(child_nodes_[i]),
        YGNodeLayoutGetTop(child_nodes_[i])};
    children_[i]->SetBounds(Rect{
        layout_result_[i].position.x,
        layout_result_[i].position.y,
        layout_result_[i].size.width,
        layout_result_[i].size.height});
  }
}

void Container::Draw(class Canvas& canvas) {
  Rect r = bounds();
  auto c = style().background();
  if (c.a > 0) {
    Paint p;
    p.SetColor(c);
    canvas.DrawRoundRect(Rect{0, 0, r.width, r.height}, style().corner_radius(), p);
  }

  for (size_t i = 0; i < children_.size(); ++i) {
    canvas.Save();
    canvas.Translate(layout_result_[i].position);
    canvas.ClipRect(Rect{
        0, 0,
        layout_result_[i].size.width,
        layout_result_[i].size.height});
    children_[i]->Draw(canvas);
    canvas.Restore();
  }
}

}  // namespace native::ui

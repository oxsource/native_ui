#include "src/framework/widgets/stack.h"
#include "src/framework/render/canvas.h"

namespace native::ui {

void Stack::ProcessArg(Children tag) {
  for (auto& child : tag.value) {
    AddChild(std::move(child));
  }
}

void Stack::ProcessArg(Id tag) { SetId(std::move(tag.value)); }

void Stack::AddChild(std::unique_ptr<Widget> child) {
  children_.push_back(std::move(child));
  RequestLayout();
}

void Stack::RemoveChild(Widget* child) {
  for (size_t i = 0; i < children_.size(); ++i) {
    if (children_[i].get() == child) {
      children_.erase(children_.begin() + static_cast<ptrdiff_t>(i));
      RequestLayout();
      return;
    }
  }
}

void Stack::ClearChildren() {
  children_.clear();
  RequestLayout();
}

Widget* Stack::ChildAt(int index) {
  if (index < 0 || static_cast<size_t>(index) >= children_.size()) {
    return nullptr;
  }
  return children_[static_cast<size_t>(index)].get();
}

int Stack::ChildCount() const {
  return static_cast<int>(children_.size());
}

int Stack::IndexOf(Widget* child) const {
  for (size_t i = 0; i < children_.size(); ++i) {
    if (children_[i].get() == child) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void Stack::Draw(Canvas& canvas) {
  Rect bb = bounds();
  for (auto& child : children_) {
    child->SetBounds(bb);
    if (!child->needs_layout() && !child->needs_draw()) {
      continue;
    }
    canvas.Save();
    child->Draw(canvas);
    canvas.Restore();
  }
}

}  // namespace native::ui

#include "hit_tester.h"

namespace native::ui {

HitTestResult HitTester::Test(Widget* root, Point point) {
  if (!root) return HitTestResult{};
  if (!root->bounds().Contains(point)) return HitTestResult{};

  // Root contains the point — compute local coordinates
  HitTestResult best;
  best.widget = root;
  best.local_pos = Point{point.x - root->bounds().x, point.y - root->bounds().y};

  // DFS into children — reverse order for topmost-first hit priority
  // This ensures that for overlapping siblings (Stack z-order, Container draw order),
  // the topmost widget is tested and matched before lower siblings.
  for (int i = root->ChildCount() - 1; i >= 0; --i) {
    Widget* child = root->ChildAt(i);
    if (!child) continue;

    // Transform point to child's parent-relative space
    // child->bounds() are relative to root, point is relative to root
    Point child_point{
      point.x - child->bounds().x,
      point.y - child->bounds().y};

    HitTestResult child_result = Test(child, child_point);
    if (child_result.widget) {
      // Found a deeper widget — prefer it (deepest = most specific)
      best = child_result;
    }
  }

  return best;
}

}  // namespace native::ui

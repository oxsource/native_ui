#include "hit_tester.h"

namespace native::ui {

static HitTestResult TestLocal(Widget* root, Point point) {
  // Check point in local bounds (origin at 0,0)
  if (point.x < 0 || point.y < 0 ||
      point.x > root->bounds().width || point.y > root->bounds().height) {
    return HitTestResult{};
  }

  HitTestResult best;
  best.widget = root;
  best.local_pos = point;

  // DFS children in reverse (topmost-first)
  for (int i = root->ChildCount() - 1; i >= 0; --i) {
    Widget* child = root->ChildAt(i);
    if (!child) continue;

    // Transform point to child's local space:
    // child->bounds() are in root's space, point is in root's space
    Point child_point{
      point.x - child->bounds().x,
      point.y - child->bounds().y};

    // Child's bounds in parent space — check before recursing
    if (child_point.x < 0 || child_point.y < 0 ||
        child_point.x > child->bounds().width || child_point.y > child->bounds().height) {
      continue;
    }

    HitTestResult child_result = TestLocal(child, child_point);
    if (child_result.widget) {
      best = child_result;
    }
  }

  return best;
}

HitTestResult HitTester::Test(Widget* root, Point point) {
  if (!root) return HitTestResult{};

  // Transform point to root's local space
  Point local_point{
    point.x - root->bounds().x,
    point.y - root->bounds().y};

  return TestLocal(root, local_point);
}

}  // namespace native::ui

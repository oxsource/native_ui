# Event System Contract

**Purpose**: Define EventHub, HitTester, DispatchResult, and event type contracts for Phase 7.

## DispatchResult

```cpp
namespace native::ui {

enum class DispatchStatus {
  kHandled,      // At least one widget handled the event
  kUnhandled,    // Reached root without being handled
  kRejected,     // Rejected by a filter before dispatch
  kNoTarget,     // No widget found at event position (mouse/touch)
};

struct DispatchResult {
  DispatchStatus status;
  Widget* target = nullptr;
};

}  // namespace native::ui
```

## Event Types

```cpp
namespace native::ui {

struct MouseButton {
  int value;
  static constexpr int kLeft = 0;
  static constexpr int kRight = 1;
  static constexpr int kMiddle = 2;
};

struct ModifierFlags {
  int value;
  static constexpr int kNone   = 0;
  static constexpr int kShift  = 1 << 0;
  static constexpr int kCtrl   = 1 << 1;
  static constexpr int kAlt    = 1 << 2;
  static constexpr int kMeta   = 1 << 3;
};

struct MouseEvent {
  Point position;
  int button = 0;       // MouseButton value
  int modifiers = 0;    // ModifierFlags bitmask
};

struct KeyEvent {
  int key_code = 0;
  int modifiers = 0;
};

}  // namespace native::ui
```

## HitTester

```cpp
namespace native::ui {

struct HitTestResult {
  Widget* widget = nullptr;
  Point local_pos;
};

class HitTester {
public:
  HitTestResult Test(Widget* root, Point point);

private:
  HitTestResult TestChildren(Widget* parent, Point point);
};

}  // namespace native::ui
```

**Contract**:
- `Test(root, point)` performs DFS, returning deepest widget whose `bounds().Contains(point)` is true
- For Container: children are tested in last-first order (topmost drawn = topmost hit)
- For Stack: children are tested in reverse order (children_[N] first = topmost)
- `local_pos` is `point - bounds().origin` of the matched widget
- Returns `{nullptr, {0,0}}` if no widget contains the point

## EventHub

```cpp
namespace native::ui {

using EventFilter = std::function<bool(const MouseEvent&)>;

class EventHub {
public:
  DispatchResult Push(const MouseEvent& event);
  DispatchResult Push(const KeyEvent& event);

  void AddFilter(EventFilter filter);
  HitTester& hit_tester();

private:
  std::vector<EventFilter> filters_;
  HitTester hit_tester_;
};

}  // namespace native::ui
```

**Dispatch Protocol**:
1. **Filter phase**: Evaluate `filters_` in registration order. Any filter returns `false` → return `{kRejected, nullptr}`.
2. **Hit test phase**: `hit_tester_.Test(root, event.position)` → no target → return `{kNoTarget, nullptr}`.
3. **Capture phase**: Walk from root → target widget (no widget handlers called, reserved for future).
4. **Target phase**: Call handler on target widget. Handler returns `true` → `{kHandled, target}`.
5. **Bubble phase**: Walk from target → root, calling handler on each ancestor. First `true` → `{kHandled, widget}`. Root reached without `true` → `{kUnhandled, target}`.

**Contract**:
- `Push(MouseEvent)` — full dispatch pipeline (filter → hit test → capture → target → bubble)
- `Push(KeyEvent)` — dispatched to currently focused widget (or root); no hit test
- `AddFilter(EventFilter)` — appends filter to the chain
- `Push` and `AddFilter` are main-thread only

## Widget Event Handling

```cpp
namespace native::ui {

class Widget {
public:
  // Event handlers — return true to stop propagation
  virtual bool OnMouseEvent(const MouseEvent& event);
  virtual bool OnKeyEvent(const KeyEvent& event);
};

}  // namespace native::ui
```

**Contract**:
- Widgets override `OnMouseEvent` or `OnKeyEvent` to handle events
- Default implementation returns `false` (unhandled)
- Button overrides `OnMouseEvent` to check `HitTest` and invoke `OnClick` callback
- Keyboard focus is implicit (deepest widget receives key events; enhanced in post-MVP)

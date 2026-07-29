# Event Interface Contract

**Purpose**: Define event types, EventHub entry point, DispatchResult feedback, hit testing, dispatch protocol, and filter mechanism.

## Design Principle

Events are **externally injected** via `EventHub::Push()` — a single unified entry point. native_ui does not own platform event loops, windows, or input capture. The consumer sources events from the platform and pushes them in; the hub returns a `DispatchResult` indicating whether the event was handled, unhandled, or rejected.

```
Consumer                         native_ui
────────                         ─────────
Platform Event Loop ──► EventHub::Push(event) → DispatchResult
                          ├── Filter chain (inspect/reject)
                          ├── HitTest
                          ├── Capture → Target → Bubble
                          └── return result to consumer
```

## Event Types

| Event | Trigger | Payload |
|-------|---------|---------|
| `MouseEvent` | Mouse click/move | position, button, modifiers |
| `KeyEvent` | Keyboard press | key_code, modifiers |
| `TouchEvent` | Touch begin/move/end | position, finger_id, pressure |

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
  Widget* target = nullptr;  // deepest widget that received the event
};

}  // namespace native::ui
```

## EventHub

```cpp
namespace native::ui {

class EventHub {
public:
  // Single entry point for external events.
  // Returns DispatchResult so the consumer can react accordingly
  // (e.g., play system alert on kUnhandled, forward rejected events).
  DispatchResult Push(const MouseEvent& event);
  DispatchResult Push(const KeyEvent& event);
  DispatchResult Push(const TouchEvent& event);

  using EventFilter = std::function<bool(const MouseEvent&)>;
  void AddFilter(EventFilter filter);

  HitTester& hit_tester() { return hit_tester_; }
};

}  // namespace native::ui
```

## Hit Testing

```cpp
class HitTester {
public:
  HitTestResult Test(Widget* root, Point point);
};

struct HitTestResult {
  Widget* widget;   // deepest widget at point, or null
  Point local_pos;  // point in widget's local coordinate space
};
```

## Dispatch Protocol

```
1. Filter phase:  registered filters inspect event, any reject → kRejected
2. Capture phase: root → leaf (non-bubbling)
3. Target phase:  the leaf widget that was hit; no hit → kNoTarget
4. Bubble phase:  leaf → root; no handler returned true → kUnhandled
```

- Event handlers return `true` to stop propagation → kHandled
- Filters enable: keyboard shortcuts, global gestures, debug overlays

## Thread Safety

- `Push()` and `AddFilter()` are **main-thread only**
- Consumer must post events from platform threads to the main thread

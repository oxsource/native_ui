# Event Interface Contract

**Last Updated**: 2026-07-29

## Design Principle

Events are **externally injected** — native_ui does not own the platform event loop, window management, or input capture. The consumer sources events from the platform and pushes them into an `EventHub` — a single entry point that handles dispatch (hit test → bubble/capture), filtering, and rejection internally.

```
Consumer (owns platform)        native_ui (pure library)
───────────────────────         ───────────────────────
Platform Event Loop
  ├── NSEvent / xcb_event
  ├── Convert to native::ui::Event
  └── EventHub::Push(event)
                                      │
                                      ▼
                              Filter → Reject?
                                      │ accepted
                                      ▼
                              HitTest → Capture → Target → Bubble
```

## Event Types

```cpp
namespace native::ui {

struct MouseEvent {
  Point position;
  MouseButton button;      // kLeft, kRight, kMiddle
  bool shift : 1;
  bool ctrl  : 1;
  bool alt   : 1;
};

struct KeyEvent {
  int key_code;
  bool shift : 1;
  bool ctrl  : 1;
  bool alt   : 1;
};

struct TouchEvent {
  Point position;
  int finger_id;
  float pressure;
};

}  // namespace native::ui
```

## DispatchResult

```cpp
namespace native::ui {

enum class DispatchStatus {
  kHandled,      // Event was handled by at least one widget handler
  kUnhandled,    // Event reached root without being handled
  kRejected,     // Event was rejected by a filter before dispatch
  kNoTarget,     // No widget was found at the event position (mouse/touch only)
};

struct DispatchResult {
  DispatchStatus status;
  Widget* target = nullptr;  // the deepest widget that received the event
};

}  // namespace native::ui
```

## EventHub

```cpp
namespace native::ui {

class EventHub {
public:
  // Push an event into the hub — the single external entry point.
  // Internally: Filter → Reject? → HitTest → Capture → Target → Bubble.
  // Must be called on the main thread.
  //
  // Returns DispatchResult so the consumer can:
  //   - Know if the event was handled, unhandled, or rejected
  //   - Access the target widget for additional processing
  //   - Play system sounds for unhandled events (e.g. macOS thump)
  //   - Forward unhandled events to the platform's default handler
  DispatchResult Push(const MouseEvent& event);
  DispatchResult Push(const KeyEvent& event);
  DispatchResult Push(const TouchEvent& event);

  // Register a filter to inspect/reject events before dispatch.
  // If any filter returns false (kRejected), the event is discarded.
  // Filters run in registration order.
  using EventFilter = std::function<bool(const MouseEvent&)>;
  void AddFilter(EventFilter filter);

  // Access the underlying hit tester (for querying without dispatching).
  HitTester& hit_tester() { return hit_tester_; }

private:
  HitTester hit_tester_;
  std::vector<EventFilter> filters_;
};

}  // namespace native::ui
```

## Hit Testing

```cpp
namespace native::ui {

class HitTester {
public:
  // DFS: returns the deepest widget containing the point
  HitTestResult Test(Widget* root, Point point);
};

struct HitTestResult {
  Widget* widget;    // deepest widget at point, or null
  Point local_pos;   // point in widget's local coordinate space
};

}  // namespace native::ui
```

## Dispatch Protocol

```
1. Filter phase:    registered filters inspect event, any reject → discard
2. Capture phase:   root → leaf (non-bubbling)
3. Target phase:    the deepest widget that was hit
4. Bubble phase:    leaf → root
```

- Event handlers return `true` to stop propagation
- Unhandled events bubble to root then are discarded
- Container widgets receive events for all their descendants (bubble up)
- Filters can inspect/modify/reject events before any widget sees them
- Filters are useful for: keyboard shortcuts, global gestures, debug overlays

## Thread Safety

- `Push()` must be called on the **main thread only**
- `AddFilter()` is main-thread only (called during setup)
- The consumer is responsible for posting events from platform threads to the main thread

## Consumer Example

```cpp
using namespace native::ui;

EventHub hub;

// Register a global filter (e.g., Cmd+Q to quit)
hub.AddFilter([](const KeyEvent& event) {
  if (event.key_code == KEY_Q && event.ctrl) {
    QuitApplication();
    return false;  // reject — already handled
  }
  return true;     // accept — continue dispatch
});

// On macOS, in NSApplication event loop:
void OnNativeEvent(NSEvent* ns) {
  if (ns.type == NSEventTypeLeftMouseDown) {
    MouseEvent event;
    event.position = Point(ns.locationInWindow.x, ns.locationInWindow.y);
    event.button = MouseButton::kLeft;
    DispatchResult result = hub.Push(event);

    switch (result.status) {
      case DispatchStatus::kHandled:
        // Widget consumed the event — platform should not beep
        break;
      case DispatchStatus::kUnhandled:
        // No widget handled it — platform may play system alert
        NSBeep();
        break;
      case DispatchStatus::kRejected:
        // Filter intercepted (e.g., Cmd+Q shortcut)
        break;
      case DispatchStatus::kNoTarget:
        // Clicked on empty area — could focus nothing
        break;
    }
  }
}
```

## Adding New Event Types

1. Define a new struct with event-specific payload
2. Add a `Push()` overload to `EventHub`
3. Add an `EventFilter` type alias for the new event type
4. The consumer converts platform events to the new type and calls `hub.Push()`

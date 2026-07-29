# Event Interface Contract

**Last Updated**: 2026-07-29

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

```text
1. Capture phase: root → leaf (non-bubbling)
2. Target phase: the deepest widget that was hit
3. Bubble phase: leaf → root
```

- Event handlers return `true` to stop propagation
- Unhandled events bubble to root then are discarded
- Container widgets receive events for all their descendants (bubble up)

## Hit Test Order

1. DFS traversal from root
2. Test each widget's bounding rect against event point
3. Return the deepest (last in DFS order) matching widget
4. If no match, return null

## Adding New Event Types

1. Define a new struct in `event.h` with event-specific payload
2. Add an `EventType` enum entry
3. Handle in the platform input adapter

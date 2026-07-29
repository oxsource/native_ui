# Event Interface Contract

**Purpose**: Define event types, hit testing, and dispatch protocol.

## Event Types

| Event | Trigger | Payload |
|-------|---------|---------|
| `MouseEvent` | Mouse click/move | position, button, modifiers |
| `KeyEvent` | Keyboard press | key_code, modifiers |
| `TouchEvent` | Touch begin/move/end | position, finger_id, pressure |

## Hit Testing

```cpp
class HitTester {
public:
  // DFS: returns the deepest widget containing the point
  HitTestResult Test(Widget* root, Point point);
};

struct HitTestResult {
  Widget* widget;   // deepest widget at point, or null
  Point local_pos;  // point in widget's coordinate space
};
```

## Dispatch Protocol

```
1. Capture phase: root → leaf (non-bubbling)
2. Target phase: the leaf widget that was hit
3. Bubble phase: leaf → root
```

- Event handlers return `true` to stop propagation
- Unhandled events bubble to root then are discarded
- Container widgets receive events for all their descendants (bubble up)

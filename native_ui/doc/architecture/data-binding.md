# Data Binding: React-Inspired State Pattern

**Last Updated**: 2026-07-29

## Overview

Data binding follows React's unidirectional data flow model:

```
         User Input (click, key, etc.)    External Data (network, I/O)
                │                                    │
                ▼                                    ▼
   ┌─────────────────────┐              ┌─────────────────────┐
   │   Widget (props ↓)      │   │   State (data)      │
   │   Event (events ↑)  │─────────────►│  Property Change    │
   └─────────────────────┘              │  Notification       │
                ▲                       └─────────────────────┘
                │                                    │
                └──────────── RequestRedraw ─────────┘
```

## State Base Class

```cpp
namespace native::ui {

class State {
public:
  virtual ~State() = default;

  // Named setter — thread-safe, can be called from worker threads
  template<typename T>
  void Set(const std::string& key, T value);

  // Operator[] + proxy assignment — syntactic sugar for Set()
  // Usage: state["count"] = 42;
  template<typename T>
  PropertyProxy<T> operator[](const std::string& key);

  // Watch a property — widget auto-redraws on change
  void Watch(Widget* widget, const std::string& property_key);

  // Unwatch — widget no longer receives notifications
  void Unwatch(Widget* widget);

  // Signal watchers of a changed property (called internally by Set/operator[])
  void Signal(const std::string& key);
};

// Proxy helper enabling state["key"] = value syntax
template<typename T>
class PropertyProxy {
public:
  void operator=(const T& value);  // calls State::Set internally
};

}  // namespace native::ui
```

## Batch Model (React-Style setState Coalescing)

Multiple property changes within a single frame are automatically batched:

```
State state;
state["count"] = 1;    // operator[] → proxy → Set → mark dirty
state["count"] = 2;    // mark dirty (overwrite previous)
state["name"] = "x";   // mark dirty
         ↓  (end of frame, batch flush)
   One RequestRedraw
   One Layout + Render pass
```

No explicit `nextTick` or `flushSync` is required — the frame loop naturally coalesces all pending changes.

## Watch Lifecycle

| Phase | Action | Description |
|-------|--------|-------------|
| Watch | Widget subscribes to State property | Widget stores weak reference, State records watch |
| Update | State property changes | State queues notification, delivered on main thread at next frame |
| Redraw | Widget receives notification | Widget calls RequestRedraw (or RequestLayout if size may change) |
| Unwatch | Widget unsubscribes | Called automatically on Widget::OnUnmount |
| Destroy | State destroyed | All watching widgets are notified to unwatch (weak refs invalidated) |

## Thread Safety

- `Set()` / `operator[]` is thread-safe — can be called from worker threads
- Property change notification is **always delivered on the main thread**
- Widgets must not mutate State properties during Draw()
- Use `PostTask(callback)` to schedule work on the main thread if needed

## Comparison with React

| React Concept | native_ui Equivalent |
|---------------|---------------------|
| `useState` / `useReducer` | `State` with `Set()` / `operator[]` |
| `setState` batching | Automatic frame-level batch coalescing |
| `useEffect` | PostNextFrame callback |
| Props (child parameters) | Tagged parameters in widget constructors |
| Events (child→parent) | Event bubbling via HitTester + Event dispatch |
| `useCallback` / `useMemo` | Plain C++ function caching (no hooks) |

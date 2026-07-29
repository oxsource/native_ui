# Data Binding: React-Inspired State Pattern

**Last Updated**: 2026-07-29

## Overview

Data binding follows React's unidirectional data flow model:

```
         User Input (click, key, etc.)    External Data (network, I/O)
                │                                    │
                ▼                                    ▼
   ┌─────────────────────┐              ┌─────────────────────┐
   │   Widget (props ↓)  │              │  State (data)       │
   │   Event (events ↑)  │─────────────►│  Property Change    │
   └─────────────────────┘              │  Notification       │
                ▲                       └─────────────────────┘
                │                                    │
                └──────────── RequestRedraw ─────────┘
```

## Property<T> and State

```cpp
namespace native::ui {

// Non-template base for heterogeneous storage in State
class PropertyBase {
public:
  virtual ~PropertyBase() = default;
  virtual void Signal() = 0;
  virtual PropertyBase* key() const { return const_cast<PropertyBase*>(this); }
};

// Typed property with value storage, change notification, and extension hooks
template<typename T>
class Property : public PropertyBase {
public:
  Property(State* owner) : owner_(owner) {}

  Property& operator=(const T& val) {
    if (before_set_) before_set_(val);
    value_ = val;
    Signal();
    if (after_set_) after_set_(val);
    return *this;
  }

  const T& value() const { return value_; }
  operator const T&() const { return value_; }

  // Extension hooks for proxy/intercept
  void OnBeforeSet(std::function<void(const T&)> fn) { before_set_ = std::move(fn); }
  void OnAfterSet(std::function<void(const T&)> fn) { after_set_ = std::move(fn); }

  void Signal() override { owner_->NotifyWatchers(key(), &value_); }

private:
  State* owner_;
  T value_;
  std::function<void(const T&)> before_set_;
  std::function<void(const T&)> after_set_;
};

// State base — holds watchers, triggers batch RequestRedraw
class State {
public:
  virtual ~State() = default;

  // Watch a property — widget auto-redraws on change
  // Usage: text1->Watch(state->count);
  template<typename T>
  void Watch(Widget* widget, Property<T>& prop);

  void Unwatch(Widget* widget);

private:
  friend class PropertyBase;
  void NotifyWatchers(PropertyBase* key, void* value_ptr);
};

// Widget
class Widget {
public:
  // Subscribe to a State property
  template<typename T>
  void Watch(Property<T>& prop);
};

}  // namespace native::ui
```

## Usage Example

```cpp
// Define state with typed properties — no strings anywhere
class CounterState : public State {
public:
  Property<int> count{this};
  Property<std::string> name{this};
};

auto state = std::make_shared<CounterState>();
auto text1 = std::make_unique<Text>();
auto text2 = std::make_unique<Text>();

// Watch via Property reference — compile-time type safe
text1->Watch(state->count);
text2->Watch(state->count);
text1->Watch(state->name);

// Assign — triggers Signal → batch → RequestRedraw
state->count = 42;     // operator= → before_set_ hook → Signal → watchers notified
state->name = "hello";

// Extension: value interception
state->count.OnBeforeSet([](const int& val) {
  if (val < 0) return;  // reject negative values
});
```

## Batch Model (React-Style setState Coalescing)

Multiple property changes within a single frame are automatically batched:

```
state->count = 1;      // mark dirty
state->count = 2;      // mark dirty (overwrite previous)
state->name = "x";     // mark dirty
         ↓  (end of frame, batch flush)
   One RequestRedraw
   One Layout + Render pass
```

No explicit `nextTick` or `flushSync` is required — the frame loop naturally coalesces all pending changes.

## Watch Lifecycle

| Phase | Action | Description |
|-------|--------|-------------|
| Watch | Widget subscribes to `Property<T>&` | Widget stores weak reference, State records watch by Property pointer |
| Update | `Property<T>::operator=` fires | Property calls `Signal()`, State queues notification |
| Redraw | Widget receives notification | Widget calls RequestRedraw (or RequestLayout if size may change) |
| Unwatch | Widget unsubscribes | Called automatically on Widget::OnUnmount |
| Destroy | State destroyed | All watching widgets notified, weak refs invalidated |

## Thread Safety

- `Property<T>::operator=` is thread-safe — can be called from worker threads
- Property change notification is **always delivered on the main thread**
- Widgets must not mutate State properties during Draw()
- Use `PostTask(callback)` to schedule work on the main thread if needed

## Extension Points (Future)

`Property<T>` provides natural interception points without changing the framework:

| Hook | Purpose |
|------|---------|
| `OnBeforeSet` | Validation, rejection, logging, debounce |
| `OnAfterSet` | Side effects, derived state update, serialization |
| Subclass Property<T> | Computed/read-only properties, value transformation |

## Comparison with React

| React Concept | native_ui Equivalent |
|---------------|---------------------|
| `useState` / `useReducer` | `Property<T>` member variables |
| `setState` batching | Automatic frame-level batch coalescing |
| `useEffect` | PostNextFrame callback |
| Props (child parameters) | Tagged parameters in widget constructors |
| Events (child→parent) | Event bubbling via HitTester + Event dispatch |
| `useCallback` / `useMemo` | Plain C++ function caching (no hooks) |

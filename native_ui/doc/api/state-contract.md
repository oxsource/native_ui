# State Module Contract

**Last Updated**: 2026-07-29

## State Base Class

```cpp
namespace native::ui {

class State {
public:
  virtual ~State() = default;

  template<typename T>
  void Watch(Widget* widget, Property<T>& prop);

  void Unwatch(Widget* widget);
};

}  // namespace native::ui
```

## Property\<T\> Template

```cpp
namespace native::ui {

class PropertyBase {
public:
  virtual ~PropertyBase() = default;
  virtual void Signal() = 0;
  PropertyBase* key() const { return this; }
};

template<typename T>
class Property : public PropertyBase {
public:
  Property(State* owner);

  // Assignment triggers automatic Signal + batch RequestRedraw
  Property& operator=(const T& val);

  const T& value() const;
  operator const T&() const;

  // Extension hooks for value interception
  void OnBeforeSet(std::function<void(const T&)> fn);
  void OnAfterSet(std::function<void(const T&)> fn);

  void Signal() override;
};

}  // namespace native::ui
```

## Watch Lifecycle

```cpp
using namespace native::ui;

class CounterState : public State {
public:
  Property<int> count{this};
  Property<std::string> name{this};
};

auto state = std::make_shared<CounterState>();
auto text = std::make_unique<Text>();

text->Watch(state->count);   // subscribe — compile-time type safe
state->count = 42;           // assign → Signal → batch → RequestRedraw
```

| Phase | Action | Description |
|-------|--------|-------------|
| Watch | Widget subscribes to `Property<T>&` | State records watch by Property pointer |
| Update | `Property<T>::operator=` fires | Property calls `Signal()`, State queues notification |
| Redraw | Widget receives notification | Widget calls RequestRedraw (or RequestLayout if size changes) |
| Unwatch | Widget unsubscribes | Called automatically on Widget::OnUnmount |

## Extension Hooks

```cpp
state->count.OnBeforeSet([](const int& val) {
  if (val < 0) val = 0;  // clamp negative values
});

state->count.OnAfterSet([](const int& val) {
  // log, trigger derived state, etc.
});
```

## Thread Safety

- `Property<T>::operator=` is thread-safe (mutex-protected inside State)
- Property change notification is **always delivered on the main thread**
- Widgets must not mutate State properties during Draw()

## Worker → Main Thread Protocol

```
Worker Thread:                     Main Thread:
  state->count = 42;                frame loop step 3:
    → mutex lock                      → Batch State changes
    → update value                    → coalesce → RequestRedraw
    → Signal()                        → Layout + Render
    → queue notificaton             frame loop step 6:
    → mutex unlock                    → PostNextFrame callbacks
```

## LogSlot Registration

```cpp
using namespace native::ui;

// Register at application startup
SetLogSlot(new StderrLogSlot());
// Null = no-op (zero overhead when logging not configured)
SetLogSlot(nullptr);
```

## Rules

- State must outlive all watching widgets (use `shared_ptr`)
- Widgets auto-unwatch on `OnUnmount` — no manual cleanup needed
- `OnBeforeSet` can reject values but must not throw
- Extension hooks are called on the thread that writes the property

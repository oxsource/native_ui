# State + Property\<T\> Contract

## Property\<T\>

```cpp
template<typename T>
class Property : public PropertyBase {
public:
  Property(State* owner);

  Property& operator=(const T& val);
  const T& value() const;
  operator const T&() const;

  void OnBeforeSet(std::function<void(const T&)> fn);
  void OnAfterSet(std::function<void(const T&)> fn);

  void Signal() override;
};
```

## State

```cpp
namespace native::ui {

class State {
public:
  virtual ~State();

  // Called by Widget::Watch — registers a watcher for a Property
  void AddWatcher(Widget* widget, PropertyBase* prop);

  // Called by Widget::UnwatchAll — removes all watchers for this widget
  void RemoveWatcher(Widget* widget);

protected:
  void NotifyWatchers(PropertyBase* key, void* value_ptr);
};

}  // namespace native::ui
```

## Usage

```cpp
using namespace native::ui;

class CounterState : public State {
public:
  Property<int> count{this};
};

auto state = std::make_shared<CounterState>();
auto widget = std::make_unique<Container>();

widget->Watch(state->count);   // Watch on Widget, not on State
state->count = 42;             // triggers notification → RequestRedraw
```

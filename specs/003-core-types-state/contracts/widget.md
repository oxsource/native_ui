# Widget + Container Contract

## Widget

```cpp
namespace native::ui {

class Widget {
public:
  virtual ~Widget() = default;

  void SetId(std::string id);
  const std::string& GetId() const;

  Widget* FindById(const std::string& id);
  virtual Widget* ChildAt(int index);
  virtual int ChildCount() const;
  virtual int IndexOf(Widget* child) const;

  void RequestLayout();
  void RequestRedraw();

  // Data binding — subscribe to a State property
  template<typename T>
  void Watch(Property<T>& prop);

  void UnwatchAll();  // called automatically on OnUnmount

  virtual void OnMount();
  virtual void OnUnmount();
  virtual void Draw(Canvas&) = 0;

private:
  std::string id_;
  bool needs_layout_ = false;
  bool needs_draw_ = false;
};

}  // namespace native::ui
```

## Container

```cpp
class Container : public Widget {
public:
  template <typename... Args>
  explicit Container(Args&&... args);

  struct Children { std::vector<std::unique_ptr<Widget>> value; };

  void AddChild(std::unique_ptr<Widget> child);
  void RemoveChild(Widget* child);
  void ClearChildren();

  Widget* ChildAt(int index) override;
  int ChildCount() const override;

private:
  void ProcessArg(Direction tag);
  void ProcessArg(Padding tag);
  void ProcessArg(Gap tag);
  void ProcessArg(Children tag);
  void ProcessArg(Id tag);

  std::vector<std::unique_ptr<Widget>> children_;
  FlexLayout layout_;  // forward-declared
};
```

## Tagged-Parameter Convention

```cpp
auto container = Container(
    Direction(kRow),
    Gap(8),
    Padding(12),
    Children{
        std::make_unique<Text>(Content("Hello")),
        std::make_unique<Button>(Id("ok"), Label("OK")),
    }
);
```

Fold expression: `(ProcessArg(std::forward<Args>(args)), *this)`

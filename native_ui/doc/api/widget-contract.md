# Widget Interface Contract

**Last Updated**: 2026-07-29

## Widget Base

```cpp
namespace native::ui {

class Widget {
public:
  virtual ~Widget() = default;

  // -- ID --
  void SetId(std::string id);
  const std::string& GetId() const;

  // -- Tree navigation --
  Widget* FindById(const std::string& id);  // DFS
  virtual Widget* ChildAt(int index);
  virtual int ChildCount() const;
  virtual int IndexOf(Widget* child) const;

  // -- Layout invalidation --
  void RequestLayout();   // triggers re-measure + re-arrange + redraw
  void RequestRedraw();   // triggers re-draw only

  // -- Lifecycle --
  virtual void OnMount();
  virtual void OnUnmount();
  virtual void OnLayout();
  virtual void Draw(Canvas&) = 0;
};

}  // namespace native::ui
```

## Extension Points

| Method | Required? | When to Override |
|--------|-----------|------------------|
| `Draw(Canvas&)` | **Yes** — pure virtual | Render widget content |
| `OnMount()` | Optional | Initialize resources, watch States |
| `OnUnmount()` | Optional | Release resources, unwatch States |
| `OnLayout()` | Optional | Custom invalidation logic on RequestLayout |
| `ChildAt(int)` | Optional | Expose children for layout/hit-test |
| `ChildCount()` | Optional | Return child count |

## Tagged-Parameter Constructor Convention

All concrete widgets use tagged-parameter constructors:

```cpp
using namespace native::ui;

// Tag types
struct Direction     { FlexDirection value; };
struct Padding       { float value; };
struct Gap           { float value; };
struct Content       { std::string value; };
struct Id            { std::string value; };

// Concrete widget
class Container : public Widget {
public:
  template <typename... Args>
  explicit Container(Args&&... args);
};

// Construction
auto row = Container(
    Direction(kRow),
    Gap(8),
    Padding(12),
    Children{
        Text(Content("Hello")),
        Button(Id("submit"), Label("OK"))
    }
);
```

## Invalidation Protocol

```
RequestLayout()
  → marks widget + ancestors layout dirty
  → at next frame: re-Measure → re-Arrange → re-Draw

RequestRedraw()
  → marks widget visual dirty
  → at next frame: re-Draw only (same layout)
```

## Rules

- Leaf widgets (no children) return `ChildCount() == 0`
- Container widgets maintain `std::vector<std::unique_ptr<Widget>> children_`
- Never delete a raw `Widget*` received from the framework
- Always call `RequestLayout()` after structural changes (add/remove children)

# Widget Interface Contract

**Purpose**: Define the Widget base class contract and extension points for custom widgets.

## Widget Base

```cpp
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
  virtual void Draw(Canvas&) = 0;

private:
  std::string id_;
};
```

## Extension Points

| Method | Override? | When to Override |
|--------|-----------|------------------|
| `Draw(Canvas&)` | **Required** | Render widget content |
| `ChildAt(int)` | Optional | Expose children for layout/hit-test |
| `ChildCount()` | Optional | Return child count |
| `IndexOf(Widget*)` | Optional | Find child index |

## Convention

- All concrete widgets use tagged-parameter constructors
- Leaf widgets (no children) return ChildCount() == 0
- Container widgets maintain a `std::vector<std::unique_ptr<Widget>> children_`

# Widget Interface Contract

**Purpose**: Define the Widget base class contract, Container ↔ FlexLayout relationship, extension points, and layout pipeline.

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
  void RequestLayout();
  void RequestRedraw();

  // -- Lifecycle --
  virtual void OnMount();
  virtual void OnUnmount();
  virtual void OnLayout();
  virtual void Draw(Canvas&) = 0;
};

}  // namespace native::ui
```

## Container Internal Structure

```cpp
namespace native::ui {

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
  void Draw(Canvas& canvas) override;

private:
  // Tag dispatch
  void ProcessArg(Direction tag);
  void ProcessArg(JustifyContent tag);
  void ProcessArg(AlignItems tag);
  void ProcessArg(FlexWrap tag);
  void ProcessArg(Gap tag);
  void ProcessArg(Padding tag);
  void ProcessArg(Margin tag);
  void ProcessArg(Children tag);
  void ProcessArg(Id tag);

  // Internal layout
  void Measure(Size available);
  void Arrange(Size container);

  std::vector<std::unique_ptr<Widget>> children_;
  std::vector<YGNodeRef> child_nodes_;
  FlexLayout layout_;
  std::vector<MeasureResult> layout_result_;
};

}  // namespace native::ui
```

## Container ↔ FlexLayout Relationship

```
Container
│
├── children_  ─── 1:1 ───  child_nodes_  (YGNodeRef)
│   (Widget*)
│
├── layout_ (FlexLayout)
│   ├── YGNodeRef root_
│   ├── ProcessArg(Direction) → YGNodeStyleSetFlexDirection
│   ├── ProcessArg(Padding)   → YGNodeStyleSetPadding
│   ├── ProcessArg(Gap)       → YGNodeStyleSetGap
│   └── ...
│
└── layout_result_ (vector<MeasureResult>)
    ├── [i].size       ← Measure()
    └── [i].position   ← Arrange()
```

## Tag → Yoga Mapping

```text
Direction(kRow)          → YGNodeStyleSetFlexDirection(root_, YGFlexDirectionRow)
Padding(16)              → YGNodeStyleSetPadding(root_, YGEdgeAll, 16)
Gap(8)                   → YGNodeStyleSetGap(root_, YGGutterAll, 8)
JustifyContent(kCenter)  → YGNodeStyleSetJustifyContent(root_, YGJustifyCenter)
AlignItems(kStretch)     → YGNodeStyleSetAlignItems(root_, YGAlignStretch)
Margin(8)                → YGNodeStyleSetMargin(root_, YGEdgeAll, 8)
```

## AddChild Internal Flow

```text
AddChild(child)
  ├─ YGNodeNew() → child_nodes_.push_back
  ├─ children_.push_back(std::move(child))
  └─ RequestLayout() → next frame: Measure → Arrange → Draw
```

## Layout Pipeline (Measure → Arrange → Draw)

### Measure

```text
Container::Measure(available)
  ├─ layout_.SetChildren(child_nodes_)
  ├─ layout_result_ = layout_.Measure(available)
  └─ for each child: child->Measure(result[i].size)
```

### Arrange

```text
Container::Arrange(size)
  ├─ layout_.Arrange(layout_result_, size)
  └─ for each child: update child.bounds_ from result[i]
```

### Draw

```text
Container::Draw(canvas)
  loop children[i]:
    canvas.Translate(result[i].position)
    canvas.ClipRect(result[i].size)
    children_[i]->Draw(canvas)
```

### Full Frame

```text
RequestLayout()
  └─ Frame Loop:
       ├─ Measure()  ← Yoga computes sizes
       ├─ Arrange()  ← Yoga computes positions
       └─ Draw()     ← recursive child draw with translated canvas
```

## Extension Points

| Method | Required? | When to Override |
|--------|-----------|------------------|
| `Draw(Canvas&)` | **Required** | Render widget content |
| `ChildAt(int)` | Optional | Expose children for layout/hit-test |
| `ChildCount()` | Optional | Return child count |

## Stack (Z-Order Widget)

Children are stacked in `children_` vector order — `children_[0]` is bottom, `children_[N]` is top. No Yoga involved.

```cpp
namespace native::ui {

class Stack : public Widget {
public:
  template <typename... Args>
  explicit Stack(Args&&... args);

  struct Children { std::vector<std::unique_ptr<Widget>> value; };
  void AddChild(std::unique_ptr<Widget> child);
  void RemoveChild(Widget* child);

  Widget* ChildAt(int index) override;
  int ChildCount() const override;
  void Draw(Canvas& canvas) override;

private:
  std::vector<std::unique_ptr<Widget>> children_;
};

}  // namespace native::ui
```

**Draw**: iterate `children_` in order → `canvas.Save()` → `child->Draw(canvas)` → `canvas.Restore()`
**Measure**: Stack size = largest child; each child fills Stack bounds by default

| Feature | Container (FlexLayout) | Stack (Z-Order) |
|---------|----------------------|-----------------|
| Engine | Yoga | None |
| Layout | Row/Column, no overlap | Stacked overlapping |
| Draw order | Layout order | children_[0]=bottom, children_[N]=top |

## Reserved: Page (Future)

```cpp
namespace native::ui {

// Reserved for navigation/routing — post-MVP
class Page : public Container {
public:
  virtual void OnPageShow();
  virtual void OnPageHide();
};

}  // namespace native::ui
```

MVP uses nested `Container` as the page equivalent.

## Conventions

- Leaf widgets return ChildCount() == 0
- Container holds `children_` vector and mirrors it as `child_nodes_` for Yoga
- `child_nodes_` must stay in sync with `children_` (size + order)
- Always call RequestLayout() after structural changes

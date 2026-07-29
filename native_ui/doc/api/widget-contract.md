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

## Container Internal Structure

```cpp
namespace native::ui {

class Container : public Widget {
public:
  template <typename... Args>
  explicit Container(Args&&... args);

  // Children management
  struct Children { std::vector<std::unique_ptr<Widget>> value; };
  void AddChild(std::unique_ptr<Widget> child);
  void RemoveChild(Widget* child);
  void ClearChildren();

  Widget* ChildAt(int index) override;
  int ChildCount() const override;
  void Draw(Canvas& canvas) override;

private:
  // Tag processing — each tagged param maps to either a layout property or a child
  void ProcessArg(Direction tag);
  void ProcessArg(JustifyContent tag);
  void ProcessArg(AlignItems tag);
  void ProcessArg(FlexWrap tag);
  void ProcessArg(Gap tag);
  void ProcessArg(Padding tag);
  void ProcessArg(Margin tag);
  void ProcessArg(Children tag);
  void ProcessArg(Id tag);

  // Internal layout pipeline
  void Measure(Size available_size);
  void Arrange(Size container_size);

  std::vector<std::unique_ptr<Widget>> children_;   // owned child widgets
  std::vector<YGNodeRef> child_nodes_;              // Yoga nodes (1:1 with children_)
  FlexLayout layout_;                               // Yoga-based layout engine
  std::vector<MeasureResult> layout_result_;        // cached layout output
};

}  // namespace native::ui
```

## Container ↔ FlexLayout Relationship

```
Container
│
├── children_  ─── 1:1 ───  child_nodes_  ─── Yoga tree
│   (Widget*)                 (YGNodeRef)
│
├── layout_ (FlexLayout)
│   ├── YGNodeRef root_       ← container-level Yoga node
│   ├── YGNodeRef children_   ← child Yoga nodes (mirrors children_)
│   ├── ProcessArg(Direction) → YGNodeStyleSetFlexDirection(root_, ...)
│   ├── ProcessArg(Padding)   → YGNodeStyleSetPadding(root_, YGEdgeAll, ...)
│   ├── ProcessArg(Gap)       → YGNodeStyleSetGap(root_, YGGutterAll, ...)
│   └── ...
│
└── layout_result_ (vector<MeasureResult>)
    ├── [i].size       ← set by Measure()
    └── [i].position   ← set by Arrange()
```

**Key rules**:

- `children_` and `child_nodes_` stay in sync — every AddChild creates a new `YGNodeRef`
- `layout_` does not own the child Yoga nodes (Container manages their lifetime)
- `layout_result_` is invalidated on every `RequestLayout()`, recalculated on next frame

## How Tagged Parameters Map to FlexLayout

When constructing a `Container`, each tagged argument routes either to a `FlexLayout` property or to internal state:

```cpp
Container(Direction(kRow), Gap(8), Padding(12), Children{...})
    │           │           │           │
    │           │           │           └─→ ProcessArg(Children) → move into children_
    │           │           └─────────────→ ProcessArg(Gap) → YGNodeStyleSetGap(root_, 8)
    │           └─────────────────────────→ ProcessArg(Direction) → YGNodeStyleSetFlexDirection(root_, kRow)
    └─────────────────────────────────────→ ProcessArg(Padding) → YGNodeStyleSetPadding(root_, 12)
```

The fold expression `(ProcessArg(std::forward<Args>(args)), *this)` dispatches each tag by type.

## Complete Layout Pipeline

### AddChild Internal Flow

```text
Container::AddChild(child)
  │
  ├─1. Create YGNodeRef for child
  │     child_node = YGNodeNew()
  │     YGNodeStyleSetWidth/Height/Margin(child_node, ...)   // from child's own props
  │
  ├─2. Store mappings
  │     child_nodes_.push_back(child_node)
  │     children_.push_back(std::move(child))
  │
  ├─3. Mark subtree dirty
  │     RequestLayout()
  │
  └─4. At next frame:
        Measure() → Arrange() → Draw()
```

### Measure Step

```cpp
void Container::Measure(Size available) {
  // 1. Pass children's Yoga nodes to FlexLayout
  layout_.SetChildren(child_nodes_);

  // 2. Run layout calculation
  layout_result_ = layout_.Measure(available);
  // Each MeasureResult: { size, position(0,0 placeholder) }

  // 3. Measure each child widget recursively
  for (size_t i = 0; i < children_.size(); i++) {
    auto* child = dynamic_cast<Widget*>(children_[i].get());
    child->Measure(layout_result_[i].size);  // propagate constraint
  }
}
```

### Arrange Step

```cpp
void Container::Arrange(Size container_size) {
  // 1. Run Yoga arrange to get final positions
  layout_.Arrange(layout_result_, container_size);

  // 2. Update each child's position bounds
  for (size_t i = 0; i < children_.size(); i++) {
    auto* child = children_[i].get();
    child->bounds_.origin = layout_result_[i].position;
    child->bounds_.size    = layout_result_[i].size;
  }
}
```

### Draw Step

```cpp
void Container::Draw(Canvas& canvas) {
  // Draw children at their computed positions
  for (size_t i = 0; i < children_.size(); i++) {
    Canvas::StateRestore _(canvas);  // RAII save/restore

    canvas.Translate(layout_result_[i].position);
    canvas.ClipRect(Rect(0, 0, layout_result_[i].size.width,
                               layout_result_[i].size.height));

    children_[i]->Draw(canvas);
  }
}
```

### Full Frame Pipeline

```text
RequestLayout()
  │
  └─→ Frame Loop (next vsync):
        │
        ├─ Container::Measure(available)
        │    ├─ layout_.Measure(available)        ← Yoga calculates sizes
        │    └─ for each child: child->Measure()   ← recursive
        │
        ├─ Container::Arrange(size)
        │    ├─ layout_.Arrange(result_, size)     ← Yoga calculates positions
        │    └─ for each child: update child bounds
        │
        └─ Renderer::Draw(root, canvas)
             └─ Container::Draw(canvas)
                  ├─ canvas.Translate(child.position)
                  └─ child->Draw(canvas)           ← recursive
```

## Invalidation Protocol

```
RequestLayout()
  → marks widget + ancestors layout dirty
  → at next frame: full Measure → Arrange → Draw pipeline

RequestRedraw()
  → marks widget visual dirty
  → at next frame: Draw only (layout unchanged, reuse layout_result_)
```

## Stack (Z-Order Widget)

`Stack` arranges children by **z-order** — children are stacked in the order they appear in the `children_` vector, with later children drawn on top of earlier ones.

```cpp
namespace native::ui {

class Stack : public Widget {
public:
  template <typename... Args>
  explicit Stack(Args&&... args);

  struct Children { std::vector<std::unique_ptr<Widget>> value; };
  void AddChild(std::unique_ptr<Widget> child);
  void RemoveChild(Widget* child);
  void ClearChildren();

  Widget* ChildAt(int index) override;
  int ChildCount() const override;
  void Draw(Canvas& canvas) override;

private:
  std::vector<std::unique_ptr<Widget>> children_;

  void ProcessArg(Children tag);
  void ProcessArg(Id tag);
  // Positioned tags (future): Left, Top, Right, Bottom
};

}  // namespace native::ui
```

### Z-Order Rules

```text
children_[0]  → 最底层 (z=0, 最先绘制)
children_[1]  → 中间层 (z=1)
children_[N]  → 最顶层 (z=N, 最后绘制, 在最上面)
```

### Draw Implementation

```cpp
void Stack::Draw(Canvas& canvas) {
  for (auto& child : children_) {
    canvas.Save();
    // Child fills the Stack bounds by default
    // (Positioned tags would translate here in the future)
    child->Draw(canvas);
    canvas.Restore();
  }
}
```

### Measure Behavior

Unlike `Container`, `Stack` does NOT use Yoga (no FlexLayout):

```text
Measure(available):
  → Stack size = largest child's size (or explicit width/height)
  → Each child fills Stack's content box by default
  → Future: Positioned(L, T, R, B) tags for per-child offset
```

### Stack vs Container

| | Container (FlexLayout) | Stack (Z-Order) |
|---|---|---|
| Layout engine | Yoga (flexbox) | None (pure z-order) |
| Child arrangement | Row or column, no overlap | Stacked, allow overlap |
| Draw order | Children in layout order | children_[0] → bottom, children_[N] → top |
| Use cases | Toolbars, lists, forms | Overlays, badges, dialogs, floating buttons |

## Reserved: Page Widget (Future)

A `Page` widget is reserved for future navigation/routing support. It is not implemented in MVP but the conceptual slot is defined:

```cpp
namespace native::ui {

// Reserved — will be added post-MVP
class Page : public Container {
public:
  virtual void OnPageShow();  // called on navigation enter
  virtual void OnPageHide();  // called on navigation leave
};

}  // namespace native::ui
```

Multiple `Container` widgets can already be composed as a single screen via nesting:

```cpp
// Currently — Container nesting serves as "page" equivalent
auto screen = Container(Direction(kColumn),
    Children{
        HeaderBar(/*...*/),
        BodyContent(/*...*/),
        BottomNav(/*...*/),
    });
```

Future `Page` will wrap this pattern and add lifecycle hooks + navigation stack.

## Rules

- Leaf widgets (no children) return `ChildCount() == 0`
- Container widgets maintain `std::vector<std::unique_ptr<Widget>> children_`
- Never delete a raw `Widget*` received from the framework
- Always call `RequestLayout()` after structural changes (add/remove children)
- `child_nodes_` must stay in sync with `children_` (same size and order)

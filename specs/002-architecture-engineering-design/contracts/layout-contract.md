# Layout Interface Contract

**Purpose**: Define the FlexLayout measure/arrange protocol, Yoga wrapping internals, and how to add new layouts.

## FlexLayout API

```cpp
namespace native::ui {

class FlexLayout {
public:
  template <typename... Args>
  explicit FlexLayout(Args&&... args);  // tagged-parameter ctor

  // Measure: given parent constraints, compute child desired sizes
  std::vector<MeasureResult> Measure(Size available_size);

  // Arrange: position children based on measured sizes
  void Arrange(std::vector<MeasureResult>& measured, Size container_size);
};

struct MeasureResult {
  Size size;
  Point position;  // filled after Arrange
};

}  // namespace native::ui
```

## Tag Parameters → Yoga Mapping

| Tag | Type | Yoga API |
|-----|------|----------|
| `Direction` | `FlexDirection` | `YGNodeStyleSetFlexDirection` |
| `JustifyContent` | `JustifyContent` | `YGNodeStyleSetJustifyContent` |
| `AlignItems` | `AlignItems` | `YGNodeStyleSetAlignItems` |
| `FlexWrap` | `FlexWrap` | `YGNodeStyleSetFlexWrap` |
| `Gap` | `float` | `YGNodeStyleSetGap` |
| `Padding` | `float` | `YGNodeStyleSetPadding(YGEdgeAll, ...)` |
| `Margin` | `EdgeInsets` | `YGNodeStyleSetMargin(YGEdgeAll, ...)` |

## Yoga Wrapping Internals

```cpp
namespace native::ui {

class FlexLayout {
private:
  YGNodeRef root_ = nullptr;
  std::vector<YGNodeRef> children_;

  // Each ProcessArg maps to a YGNodeStyleSet* call
  void ProcessArg(Direction tag) {
    YGNodeStyleSetFlexDirection(root_, static_cast<YG FlexDirection>(tag.value));
  }
  void ProcessArg(Padding tag) {
    YGNodeStyleSetPadding(root_, YGEdgeAll, tag.value);
  }
  void ProcessArg(Gap tag) {
    YGNodeStyleSetGap(root_, YGGutterAll, tag.value);
  }
};

}  // namespace native::ui
```

## Measure() Internals

```cpp
std::vector<MeasureResult> Measure(Size available) {
  // 1.  Set container constraints
  YGNodeStyleSetWidth(root_, available.width);
  YGNodeStyleSetHeight(root_, available.height);

  // 2.  Insert children into Yoga tree
  for (size_t i = 0; i < children_.size(); i++)
    YGNodeInsertChild(root_, children_[i], static_cast<int32_t>(i));

  // 3.  Calculate layout
  YGNodeCalculateLayout(root_, YGUndefined, YGUndefined, YGDirectionLTR);

  // 4.  Extract measured sizes (position filled by Arrange)
  std::vector<MeasureResult> results;
  for (auto* child : children_) {
    results.push_back({
      .size = Size(YGNodeLayoutGetWidth(child), YGNodeLayoutGetHeight(child)),
      .position = Point(0, 0)
    });
  }
  return results;
}
```

## Arrange() Internals

```cpp
void Arrange(std::vector<MeasureResult>& measured, Size container) {
  for (size_t i = 0; i < children_.size(); i++) {
    measured[i].position = Point(
      YGNodeLayoutGetLeft(children_[i]),
      YGNodeLayoutGetTop(children_[i])
    );
  }
}
```

## Yoga Node Lifecycle

```cpp
FlexLayout()  { root_ = YGNodeNew(); }
~FlexLayout() { YGNodeFreeRecursive(root_); }
// Children YGNodes created/managed by Container
// Inserted into root each Measure() call
```

## Measure / Arrange Protocol

```
Container::RequestLayout()
  └→ at next frame:
     Measure(available)
       ├→ YGNodeStyleSetWidth/Height
       ├→ YGNodeInsertChild for each
       ├→ YGNodeCalculateLayout
       └→ return child sizes
     Arrange(measured, size)
       ├→ YGNodeLayoutGetLeft/Top
       └→ child positions updated
```

## Adding a New Layout

1. Create a new layout class following `FlexLayout`'s pattern
2. Implement `Measure()` and `Arrange()` with the new algorithm
3. Integrate with `Container` via tagged parameter

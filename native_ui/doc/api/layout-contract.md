# Layout Interface Contract

**Last Updated**: 2026-07-29

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
  void Arrange(const std::vector<MeasureResult>& measured, Size container_size);
};

struct MeasureResult {
  Size size;
  Point position;  // filled after Arrange
};

}  // namespace native::ui
```

## Tag Parameters

| Tag | Type | Example | Yoga API |
|-----|------|---------|----------|
| `Direction` | `YGFlexDirection` | `Direction(kRow)` | `YGNodeStyleSetFlexDirection` |
| `JustifyContent` | `YGJustify` | `JustifyContent(kCenter)` | `YGNodeStyleSetJustifyContent` |
| `AlignItems` | `YGAlign` | `AlignItems(kStretch)` | `YGNodeStyleSetAlignItems` |
| `FlexWrap` | `YGWrap` | `FlexWrap(kWrap)` | `YGNodeStyleSetFlexWrap` |
| `Gap` | `float` | `Gap(8.0f)` | `YGNodeStyleSetGap` |
| `Padding` | `float` | `Padding(16.0f)` | `YGNodeStyleSetPadding(YGEdgeAll, ...)` |
| `Margin` | `EdgeInsets` | `Margin(8.0f)` | `YGNodeStyleSetMargin(YGEdgeAll, ...)` |

## Yoga Wrapping Internals

FlexLayout wraps Yoga's C API (YGNodeRef). The internal structure:

```cpp
namespace native::ui {

class FlexLayout {
public:
  // ... public API ...

private:
  YGNodeRef root_ = nullptr;               // cached Yoga root node
  std::vector<YGNodeRef> children_;         // cached child nodes

  // Tagged-parameter dispatch — each tag maps to a YGNodeStyleSet* call
  void ProcessArg(Direction tag) {
    YGNodeStyleSetFlexDirection(root_, tag.value);
  }
  void ProcessArg(JustifyContent tag) {
    YGNodeStyleSetJustifyContent(root_, tag.value);
  }
  void ProcessArg(Padding tag) {
    YGNodeStyleSetPadding(root_, YGEdgeAll, tag.value);
  }
  void ProcessArg(Gap tag) {
    YGNodeStyleSetGap(root_, YGGutterAll, tag.value);
  }
  void ProcessArg(Margin tag) {
    YGNodeStyleSetMargin(root_, YGEdgeAll, tag.value);
  }
  // ... ProcessArg for each supported tag
};

}  // namespace native::ui
```

## Measure() Implementation

```cpp
std::vector<MeasureResult> Measure(Size available) {
  // 1. Set container constraints
  YGNodeStyleSetWidth(root_, available.width);
  YGNodeStyleSetHeight(root_, available.height);

  // 2. Ensure all child YGNodes are inserted
  for (size_t i = 0; i < children_.size(); i++) {
    YGNodeInsertChild(root_, children_[i], static_cast<int32_t>(i));
  }

  // 3. Run Yoga layout calculation
  YGNodeCalculateLayout(root_, YGUndefined, YGUndefined, YGDirectionLTR);

  // 4. Extract measured sizes (positions filled later by Arrange)
  std::vector<MeasureResult> results;
  for (auto* child : children_) {
    results.push_back({
      .size = Size(YGNodeLayoutGetWidth(child), YGNodeLayoutGetHeight(child)),
      .position = Point(0, 0)  // placeholder
    });
  }
  return results;
}
```

## Arrange() Implementation

```cpp
void Arrange(std::vector<MeasureResult>& measured, Size container) {
  // Read final positions from Yoga layout result
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
FlexLayout() {
  root_ = YGNodeNew();
  // children_ are created by Container and passed via external API
}

~FlexLayout() {
  YGNodeFreeRecursive(root_);
}

// Called externally when children change
void SetChildren(std::vector<YGNodeRef> children) {
  children_ = std::move(children);
  // YGNode children are re-inserted on next Measure() call
}
```

## Measure / Arrange Protocol (Complete Flow)

```
Container::RequestLayout()
  └→ at next frame, Container calls FlexLayout::Measure(available)
      ├→ YGNodeStyleSetWidth/Height on root
      ├→ YGNodeInsertChild for each child
      ├→ YGNodeCalculateLayout
      └→ return child sizes

Container::Arrange(measured)
  └→ Container calls FlexLayout::Arrange(measured, size)
      ├→ YGNodeLayoutGetLeft/Top for each child
      └→ update child positions
```

## StackLayout (Non-Yoga, Z-Order)

`Stack` does NOT use Yoga or FlexLayout. It is a pure z-order container:

```text
Measure → size = largest child's size
Arrange → each child fills Stack's content box
Draw    → children_[0] (bottom) → ... → children_[N] (top)
```

No `FlexLayout` wrapper needed — `Stack` manages children as a simple list.

## Adding a New Layout

1. Create a new layout class following `FlexLayout`'s pattern
2. Implement `Measure()` and `Arrange()` with the new algorithm
3. Integrate with `Container` via tagged parameter

## Thread Safety

- Layout (Measure + Arrange) runs on the **main thread only**
- Do not access layout results from worker threads

# FlexLayout Interface Contract

## Standards Reference

FlexLayout implements a subset of the **W3C CSS Flexible Box Layout Module (Flexbox)** via the **Yoga** layout engine.

| Reference | URL |
|-----------|-----|
| W3C Flexbox Specification | https://www.w3.org/TR/css-flexbox-1/ |
| Yoga Layout Engine | https://yogalayout.dev/ |
| Yoga C API Reference | https://www.yogalayout.dev/docs/learn/c-api |

## Flexbox Algorithm Overview

FlexLayout follows the standard two-phase flexbox layout algorithm:

```text
Phase 1 — Measure (determine sizes):
  1. Set container main-axis and cross-axis size from available_size
  2. Set flex-basis on each child (default = child's main-axis size)
  3. Distribute positive/negative free space based on flex-grow / flex-shrink
  4. Determine final main-axis size for each child
  5. Determine cross-axis size based on min/max constraints and align-items

Phase 2 — Arrange (determine positions):
  1. Children are arranged along the main-axis per justify-content
     ── flex-start: children packed at start
     ── center:     children centered
     ── flex-end:   children packed at end
     ── space-between: even spacing, first/last at container edges
     ── space-around:  even spacing with half-gaps at edges
     ── space-evenly:  even spacing with equal gaps at edges
  2. Cross-axis alignment per align-items / align-self
     ── stretch:  child fills cross-axis
     ── flex-start: child packed at cross-axis start
     ── center:   child centered on cross-axis
     ── flex-end: child packed at cross-axis end
  3. If flex-wrap is enabled and children overflow, they wrap to a new line
```

Yoga implements the full W3C Flexbox algorithm including all edge cases. FlexLayout's role is to provide a C++ idiomatic wrapper over Yoga's C API — it does **not** reimplement the algorithm.

## FlexLayout API

```cpp
namespace native::ui {

struct MeasureResult {
  Size size;
  Point position;  // filled by Arrange
};

class FlexLayout {
public:
  template <typename... Args>
  explicit FlexLayout(Args&&... args);

  void SetChildren(const std::vector<YGNodeRef>& children);
  std::vector<MeasureResult> Measure(Size available_size);
  void Arrange(std::vector<MeasureResult>& measured, Size container_size);
};

}  // namespace native::ui
```

## Tag Parameters → Yoga Mapping

| Tag Name | C++ Type | Yoga API | W3C Flexbox Property |
|----------|----------|----------|---------------------|
| `Direction(kRow)` | `Direction` | `YGNodeStyleSetFlexDirection(root_, YGFlexDirectionRow)` | `flex-direction: row` |
| `JustifyContent(kCenter)` | `JustifyContent` | `YGNodeStyleSetJustifyContent(root_, YGJustifyCenter)` | `justify-content: center` |
| `AlignItems(kStretch)` | `AlignItems` | `YGNodeStyleSetAlignItems(root_, YGAlignStretch)` | `align-items: stretch` |
| `FlexWrap(kWrap)` | `FlexWrap` | `YGNodeStyleSetFlexWrap(root_, YGWrapWrap)` | `flex-wrap: wrap` |
| `Gap(8.0f)` | `Gap` | `YGNodeStyleSetGap(root_, YGGutterAll, 8)` | `gap: 8px` |
| `Padding(16.0f)` | `Padding` | `YGNodeStyleSetPadding(root_, YGEdgeAll, 16)` | `padding: 16px` |
| `Margin(8.0f)` | `Margin` | `YGNodeStyleSetMargin(root_, YGEdgeAll, 8)` | `margin: 8px` |
| `AlignContent(kCenter)` | `AlignContent` | `YGNodeStyleSetAlignContent(root_, YGAlignCenter)` | `align-content: center` |

## Per-Child Properties (via Yoga C API directly)

Some flexbox properties are set per-child rather than on the container. These are accessed by calling the Yoga API directly on each child's YGNodeRef:

| Property | Yoga API | W3C Equivalent |
|----------|----------|----------------|
| Flex grow | `YGNodeStyleSetFlexGrow(child, 1.0f)` | `flex-grow: 1` |
| Flex shrink | `YGNodeStyleSetFlexShrink(child, 1.0f)` | `flex-shrink: 1` |
| Flex basis | `YGNodeStyleSetFlexBasis(child, 100.0f)` | `flex-basis: 100px` |
| Align self | `YGNodeStyleSetAlignSelf(child, YGAlignCenter)` | `align-self: center` |
| Width | `YGNodeStyleSetWidth(child, 60.0f)` | `width: 60px` |
| Height | `YGNodeStyleSetHeight(child, 40.0f)` | `height: 40px` |
| Min/Max width | `YGNodeStyleSetMinWidth/MinHeight/MaxWidth/MaxHeight` | `min-width`, `max-height`, etc. |
| Position (absolute) | `YGNodeStyleSetPositionType(child, YGPositionTypeAbsolute)` | `position: absolute` |

## Measure() Internals

```cpp
std::vector<MeasureResult> Measure(Size available) {
  // 1. Set container constraints (W3C: container size)
  YGNodeStyleSetWidth(root_, available.width);
  YGNodeStyleSetHeight(root_, available.height);

  // 2. Insert children into Yoga tree (W3C: collect flex items)
  for (size_t i = 0; i < children_.size(); i++)
    YGNodeInsertChild(root_, children_[i], static_cast<int32_t>(i));

  // 3. Run Yoga layout algorithm (W3C: resolve flexible lengths)
  //    Yoga handles: flex-direction, flex-wrap, justify-content,
  //    align-items, align-content, flex-grow, flex-shrink, flex-basis,
  //    gap, padding, margin, min/max constraints
  YGNodeCalculateLayout(root_, YGUndefined, YGUndefined, YGDirectionLTR);

  // 4. Extract computed sizes (W3C: determine main size)
  std::vector<MeasureResult> results;
  for (auto* child : children_) {
    results.push_back({
      .size = Size(YGNodeLayoutGetWidth(child), YGNodeLayoutGetHeight(child)),
      .position = Point(0, 0)  // Position filled by Arrange
    });
  }
  return results;
}
```

## Arrange() Internals

```cpp
void Arrange(std::vector<MeasureResult>& measured, Size container) {
  // Read final positions from Yoga (W3C: cross-axis + main-axis placement)
  for (size_t i = 0; i < children_.size(); i++) {
    measured[i].position = Point(
      YGNodeLayoutGetLeft(children_[i]),
      YGNodeLayoutGetTop(children_[i])
    );
  }
}
```

## Lifecycle

```cpp
FlexLayout()  { root_ = YGNodeNew(); }
~FlexLayout() { YGNodeFreeRecursive(root_); }
// Children YGNodes created/managed by Container — not owned here
```

## Thread Safety

- `Measure()` and `Arrange()` must be called on the **main thread only**
- `SetChildren()` is main-thread only
- Yoga's `YGNodeCalculateLayout` is not thread-safe — never call from multiple threads

## Adding FlexLayout to Container

In Phase 5 (Basic Widgets), Container will use FlexLayout internally:

```cpp
class Container : public Widget {
  void RequestLayout() override {
    // FlexLayout is owned by Container
    auto results = layout_.Measure(available_size_);
    layout_.Arrange(results, available_size_);
    // Store results for Draw() — children use result[i].position
  }

  FlexLayout layout_;  // member, configured via tagged params
};
```


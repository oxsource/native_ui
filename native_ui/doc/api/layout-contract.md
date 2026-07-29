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

| Tag | Type | Example |
|-----|------|---------|
| `Direction` | `YGFlexDirection` | `Direction(kRow)` |
| `JustifyContent` | `YGJustify` | `JustifyContent(kCenter)` |
| `AlignItems` | `YGAlign` | `AlignItems(kStretch)` |
| `FlexWrap` | `YGWrap` | `FlexWrap(kWrap)` |
| `Gap` | `float` | `Gap(8.0f)` |
| `Padding` | `float` | `Padding(16.0f)` |
| `Margin` | `EdgeInsets` | `Margin(8.0f)` |

## Measure / Arrange Protocol

```
Measure(available_size)
  → For each child, compute desired size given constraints
  → Uses Yoga to resolve flexbox rules
  → Returns vector of MeasureResult

Arrange(measured, container_size)
  → Using Yoga, compute final positions for each child
  → Updates MeasureResult.position for each child
  → Widgets read positions for Draw()
```

## Adding a New Layout

1. Create a new layout class following `FlexLayout`'s pattern
2. Implement `Measure()` and `Arrange()` with the new algorithm
3. Integrate with `Container` via tagged parameter

## Thread Safety

- Layout (Measure + Arrange) runs on the **main thread only**
- Do not access layout results from worker threads

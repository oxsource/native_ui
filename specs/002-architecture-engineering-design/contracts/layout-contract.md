# Layout Interface Contract

**Purpose**: Define the FlexLayout measure/arrange protocol and how to add new layouts.

## FlexLayout API

```cpp
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
```

## Tag Parameters

| Tag | Type | Example |
|-----|------|---------|
| `Direction` | `FlexDirection` | `Direction(kRow)` |
| `JustifyContent` | `JustifyContent` | `JustifyContent(kCenter)` |
| `AlignItems` | `AlignItems` | `AlignItems(kStretch)` |
| `FlexWrap` | `FlexWrap` | `FlexWrap(kWrap)` |
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

1. Create a new layout class following FlexLayout's pattern
2. Implement `Measure()` and `Arrange()` with the new algorithm
3. Integrate with Container via tagged parameter

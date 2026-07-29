# <Widget Name> Specification

**Version**: 0.1

## Interface

```cpp
namespace native::ui {

class <Widget> : public <Base> {
public:
  template <typename... Args>
  explicit <Widget>(Args&&... args);

  // — Drawing —
  void Draw(Canvas& canvas) override;

  // — Layout —
  Widget* ChildAt(int index) override;
  int ChildCount() const override;
};

}  // namespace native::ui
```

## Behavior

```mermaid
sequenceDiagram
    participant App as Application
    participant W as Widget
    participant L as Layout
    participant R as Render
    App->>W: Create(...)
    App->>W: Draw(canvas)
    W->>R: Draw content
    R-->>App: Pixel output
```

## Edge Cases

- What happens when <condition>?
- How does it behave with <boundary value>?

## Test Points

- [ ] <testable assertion 1>
- [ ] <testable assertion 2>

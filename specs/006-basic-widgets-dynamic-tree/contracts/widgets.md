# Widget API Contract Additions (Phase 6)

**Purpose**: Define the public API contracts for Text, Button, Image, and Stack widgets added in Phase 6.

## Text Widget

```cpp
#pragma once
#include "widget.h"

namespace native::ui {

struct Content {
  std::string value;
};

struct FontSize {
  float value;
};

class Text : public Widget {
public:
  template <typename... Args>
  explicit Text(Args&&... args);

  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(Content tag);
  void ProcessArg(FontSize tag);
  void ProcessArg(Color tag);
  void ProcessArg(Id tag);

  std::string content_;
  float font_size_ = 16.0f;
  Color color_ = Color::kBlack;
  Property<std::string>* watched_prop_ = nullptr;
};

}  // namespace native::ui
```

**Contract**:
- `Text(Content("Hi"))` — sets text content
- `Text(FontSize(24))` — sets font size (clamped to >= 1)
- `Text(Id("title"))` — sets widget ID for FindById lookup
- `Watch(Property<std::string>&)` binds content to a State property — the widget reads `prop.value()` and redraws on change
- `Draw(Canvas&)` renders text at origin — position is set by parent Container's Arrange

## Button Widget

```cpp
#pragma once
#include "widget.h"

namespace native::ui {

struct Label {
  std::string value;
};

struct OnClick {
  std::function<void()> value;
};

class Button : public Widget {
public:
  template <typename... Args>
  explicit Button(Args&&... args);

  bool HitTest(Point p) const;
  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(Label tag);
  void ProcessArg(OnClick tag);
  void ProcessArg(Id tag);

  std::string label_;
  std::function<void()> on_click_;
  Rect bounds_;
  Property<std::string>* watched_prop_ = nullptr;
};

}  // namespace native::ui
```

**Contract**:
- `Button(Label("OK"))` — sets button label text
- `Button(OnClick([](){ ... }))` — sets click callback
- `Button(Id("submit"))` — sets widget ID
- `HitTest(Point)` returns true if point is within the button's layout bounds
- `on_click_()` is invoked when HitTest returns true and the consumer decides to fire
- `Watch(Property<std::string>&)` binds label to a State property

## Image Widget

```cpp
#pragma once
#include "widget.h"
#include "image.h"

namespace native::ui {

struct ImagePath {
  std::string value;
};

class ImageWidget : public Widget {
public:
  template <typename... Args>
  explicit ImageWidget(Args&&... args);

  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(ImagePath tag);
  void ProcessArg(Id tag);

  std::string path_;
  std::unique_ptr<Image> image_;  // null if load failed
};

}  // namespace native::ui
```

**Contract**:
- `ImageWidget(ImagePath("/path/to/img.png"))` — sets file path and triggers decode
- If file not found or decode fails, `image_` is null and Draw is a no-op
- `Draw(Canvas&)` renders the image at the widget's layout bounds — uses `canvas.DrawImage()`

## ExternalImage Widget

```cpp
#pragma once
#include "widget.h"
#include "hardware_buffer.h"

namespace native::ui {

class ExternalImage : public Widget {
public:
  template <typename... Args>
  explicit ExternalImage(Args&&... args);

  void SetBuffer(HardwareBuffer buffer);
  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(HardwareBuffer tag);
  void ProcessArg(Id tag);

  HardwareBuffer buffer_;
  std::unique_ptr<Image> image_;  // recreated on SetBuffer
  Property<HardwareBuffer>* watched_prop_ = nullptr;
};

}  // namespace native::ui
```

**Contract**:
- `ExternalImage(HardwareBuffer::FromIOSurface(...))` — sets initial buffer
- `SetBuffer(HardwareBuffer)` — replaces the buffer and calls `RequestRedraw()`
- `Watch(Property<HardwareBuffer>&)` — binds buffer to a State property; triggers redraw on change
- Invalid/null buffer is handled gracefully — Draw is a no-op
- Each `SetBuffer` or property change re-creates the internal Image via `Image::FromBuffer()`
- ExternalImage does NOT own the underlying platform buffer — the producer retains ownership

## Stack Widget

```cpp
#pragma once
#include "widget.h"

namespace native::ui {

class Stack : public Widget {
public:
  template <typename... Args>
  explicit Stack(Args&&... args);

  struct Children {
    std::vector<std::unique_ptr<Widget>> value;
  };

  void AddChild(std::unique_ptr<Widget> child);
  void RemoveChild(Widget* child);
  void ClearChildren();

  Widget* ChildAt(int index) override;
  int ChildCount() const override;
  int IndexOf(Widget* child) const override;
  void Draw(Canvas& canvas) override;

private:
  void ProcessArg(Children tag);
  void ProcessArg(Id tag);

  std::vector<std::unique_ptr<Widget>> children_;
};

}  // namespace native::ui
```

**Contract**:
- `Stack(Children{...})` — constructs with children in z-order
- Children drawn in order: index 0 = bottom, highest index = top
- `Stack` does NOT use Yoga — no flexbox, no measure/arrange pipeline
- Stack size = largest child size; each child fills Stack bounds
- `AddChild` and `RemoveChild` trigger `RequestLayout()`

## Layout Invalidation Rules

| Widget | `needs_layout_` trigger | `needs_draw_` trigger |
|--------|----------------------|----------------------|
| Text | — (leaf) | State property change, RequestRedraw |
| Button | — (leaf) | State property change, RequestRedraw |
| Image | — (leaf) | RequestRedraw |
| ExternalImage | — (leaf) | SetBuffer, State property change, RequestRedraw |
| Stack | AddChild, RemoveChild | RequestRedraw |

## Skia Isolation

- Widgets module (`src/framework/widgets/`) does NOT depend on Skia directly
- All rendering goes through `Canvas&` (from render module)
- Image decode is handled by existing `Image::FromFile` (from render module)
- Enforced by Bazel visibility and CI query

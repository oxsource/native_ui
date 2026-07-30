# Data / Entity Model: Basic Widgets & Dynamic Tree

**Date**: 2026-07-30

## Entity: Text

| Field | Type | Description |
|-------|------|-------------|
| `content_` | `std::string` | Text content to render |
| `font_size_` | `float` | Font size in pixels (default: 16) |
| `color_` | `Color` | Text color (default: kBlack) |
| `watched_prop_` | `Property<std::string>*` | Bound State property (optional, nullable) |

**Tagged ctor params**: `Content(string)`, `FontSize(float)`, `Color(Color)`, `Id(string)`

**Draw**: `canvas.DrawText(content_, {0, 0}, Paint().SetColor(color_))` — position determined by parent arrange.

**Data binding**: If `watched_prop_` is set, on State::Flush() the widget reads `watched_prop_->value()` and calls `RequestRedraw()`.

## Entity: Button

| Field | Type | Description |
|-------|------|-------------|
| `label_` | `std::string` | Button label text |
| `on_click_` | `std::function<void()>` | Click callback |
| `bounds_` | `Rect` | Layout bounds (assigned by parent) |
| `watched_prop_` | `Property<std::string>*` | Bound label property (optional) |

**Tagged ctor params**: `Label(string)`, `OnClick(function)`, `Id(string)`

**Hit testing**: `HitTest(Point p) -> bool` — returns `bounds_.Contains(p)`.

**Draw**: Draw a rounded rect background, then center label text via `canvas.DrawText()`.

## Entity: Image

| Field | Type | Description |
|-------|------|-------------|
| `path_` | `std::string` | File path to image |
| `image_` | `std::unique_ptr<Image>` | Decoded Skia image (null on load failure) |

**Tagged ctor params**: `ImagePath(string)`, `Id(string)`

**Draw**: If `image_` is valid, `canvas.DrawImage(*image_, bounds)`. Otherwise no-op.

**Error handling**: If file not found or decode fails, `image_` remains null and Draw is a no-op. No crash, no exception.

## Entity: ExternalImage

| Field | Type | Description |
|-------|------|-------------|
| `buffer_` | `HardwareBuffer` | Current platform hardware buffer (invalid on construction) |
| `image_` | `std::unique_ptr<Image>` | Cached Image wrapper around buffer (recreated on update) |
| `watched_prop_` | `Property<HardwareBuffer>*` | Bound buffer property (optional, nullable) |

**Tagged ctor params**: `HardwareBuffer(HardwareBuffer)`, `Id(string)`

**Draw**: If `buffer_` is valid, `canvas.DrawImage(*image_, bounds)`. Otherwise no-op.

**Buffer update**: `SetBuffer(HardwareBuffer)` re-creates the internal Image via `Image::FromBuffer()` and calls `RequestRedraw()`.

**Data binding**: If `watched_prop_` is set, on property change the widget reads the new buffer, re-creates the internal Image, and redraws.

## Entity: Stack

| Field | Type | Description |
|-------|------|-------------|
| `children_` | `vector<unique_ptr<Widget>>` | Child widgets in z-order (0=bottom, N=top) |

**Tagged ctor params**: `Children{...}`, `Id(string)`

**Draw**: Iterate `children_` in order → `canvas.Save()` → `child->Draw(canvas)` → `canvas.Restore()`.

**Size**: Stack's natural size = largest child's size. Each child is arranged to fill Stack bounds.

**Methods**: `AddChild(unique_ptr<Widget>)`, `RemoveChild(Widget*)`, `ClearChildren()`, `ChildAt(int)`, `ChildCount()`.

## Relationships

```
Widget (base)
  ├── Text            (leaf, draws text, no children)
  ├── Button          (leaf, draws label + hit area, no children)
  ├── Image           (leaf, draws decoded static image, no children)
  ├── ExternalImage   (leaf, draws hardware buffer, no children)
  ├── Container       (existing, flexbox via Yoga)
  └── Stack           (container, z-order, no Yoga)
        └── children_: Widget*  (owned via unique_ptr)
```

## Validation Rules

| Rule | Applies To | Description |
|------|-----------|-------------|
| Empty content renders no text | Text | `content_.empty()` → Draw is no-op |
| Zero/negative font size | Text | Clamp to 1px minimum |
| Empty label renders empty button | Button | Still draws background, no text |
| Nonexistent image path | Image | `image_` stays null, no crash |
| Invalid/null hardware buffer | ExternalImage | `buffer_` stays invalid, no crash |
| Buffer updated mid-frame | ExternalImage | `SetBuffer` marks dirty, new buffer renders next frame |
| Zero children | Stack | Draw is no-op, no crash |
| Data-bound property destroyed | Text, Button | Widget's `watched_prop_` set to null during UnwatchAll |

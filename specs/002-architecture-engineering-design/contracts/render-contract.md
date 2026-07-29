# Render Interface Contract

**Purpose**: Define the Surface/Image/Canvas/Paint/Path RAII wrappers and Skia isolation rules.

## Surface ↔ Canvas Relationship

```
┌─────────────────────────────────────────────────┐
│                  Surface                         │
│  (backing store — owns the pixel buffer)         │
│                                                   │
│   ┌───────────────────────────────────────────┐  │
│   │  Canvas  (drawing context)                │  │
│   │  ────────────────────────                 │  │
│   │  Constructed with Surface&                │  │
│   │  DrawRect / DrawText / DrawPath           │  │
│   │  DrawImage                                │  │
│   │  Save / Restore / ClipRect / Translate    │  │
│   │  ────────────────────────                 │  │
│   │  ~Canvas() → auto restore to save state   │  │
│   └───────────────────────────────────────────┘  │
│                                                   │
│  Surface::Flush() → commit pixels to display      │
└─────────────────────────────────────────────────┘
```

**Lifecycle**:

```text
1. Create Surface          Surface::Create(w, h)  or  Surface::CreateFromBuffer(hardwareBuffer)
2. Attach Canvas           Canvas canvas(surface)
3. Draw                    canvas.DrawRect(...); canvas.DrawText(...); canvas.DrawImage(...)
4. Destroy Canvas          ~Canvas() → auto restore SkCanvas state
5. Flush                   surface.Flush() → commit pixels to display
6. Repeat from step 2 for next frame
```

**Key contracts**:

| Role | Responsibility |
|------|---------------|
| `Surface` | Owns the pixel buffer; knows how to create from size or external `HardwareBuffer`; `Flush()` to commit pixels |
| `Canvas` | Lightweight RAII wrapper attached to a `Surface&`; provides all drawing APIs; auto save/restore on scope exit |
| One Surface → one Canvas at a time | Canvas is not shared between threads; create/destroy per frame |

## Frame Pipeline: Change → Flush

### Dirty Widget Tracking

Each widget tracks two flags:

| Flag | Means | Next Frame Action |
|------|-------|-------------------|
| `needs_draw_` | Visual content changed (same layout) | Draw only dirty widgets |
| `needs_layout_` | Structure or size changed | Full Measure → Arrange → Draw |

### Trigger → Flag Mapping

| Trigger | Flag |
|---------|------|
| State property change (same size) | `needs_draw_` |
| AddChild / RemoveChild | `needs_layout_` |
| State property change (size affected) | `needs_layout_` |
| RequestRedraw() | `needs_draw_` |
| RequestLayout() | `needs_layout_` |

### Batch Coalescing

Multiple changes within the same frame coalesce into a single draw pass:

```text
Frame N:
  state->count = 1       → mark dirty
  state->count = 2       → still dirty (same widget)
  state->name = "x"      → mark dirty
  AddChild(btn)          → mark layout dirty
           ↓  (frame boundary)
  One pass: Measure → Arrange → Draw → Flush
```

### Full Sequence

```text
 1. Change:        state->count = 42
                   → Property::operator= → Signal → RequestRedraw

 2. Mark dirty:    Widget::needs_draw_ = true
                   → propagates to root for scheduling

 3. Frame start:   consumer's clock → frame loop begins

 4. Batch:         all pending Signals coalesced → one invalidation

 5. If layout dirty:
     ├─ Container::Measure(available)   ← Yoga
     └─ Container::Arrange(size)        ← positions

 6. Render dirty widgets:
     ├─ Canvas canvas(surface)
     ├─ For each dirty widget:
     │    canvas.Translate(pos)
     │    widget->Draw(canvas)
     └─ ~Canvas() → auto restore

 7. Flush:         surface.Flush() → pixels committed

 8. Clear flags:   needs_draw_ = needs_layout_ = false
```

### Partial Draw

If only `needs_draw_` is set, the frame loop skips clean widgets:

```cpp
for (auto& child : children_) {
  if (!child->needs_layout_ && !child->needs_draw_) continue;
  canvas.Save();
  canvas.Translate(child->position);
  child->Draw(canvas);
  canvas.Restore();
  child->needs_draw_ = child->needs_layout_ = false;
}
```

## Canvas Sharing Across Widget Tree

One `Canvas` per frame, shared by all widgets via `save/translate/restore`.

```
Surface (pixel buffer)
  └── Canvas canvas(surface)     ← single canvas for this frame
       ├── canvas.Save()
       ├── canvas.Translate(child.position)
       ├── child->Draw(canvas)   ← same canvas, local coords
       ├── canvas.Restore()
       └── ... loop children

~Canvas() → auto restore
Surface::Flush() → commit
```

Widgets never create their own Canvas or Surface. This matches Android View and Flutter models.

## Surface (Backing Store)

```cpp
namespace native::ui {

class Surface {
public:
  static std::unique_ptr<Surface> Create(int width, int height);
  static std::unique_ptr<Surface> CreateFromBuffer(HardwareBuffer handle);
  ~Surface();

  void Flush();  // commit pixels / swap buffers for platform surfaces
  int width() const;
  int height() const;
};

}  // namespace native::ui
```

## Image (Drawable Source)

```cpp
namespace native::ui {

class Image {
public:
  // Decode from encoded data (PNG, JPEG, WebP, SVG — auto-detect format)
  static std::unique_ptr<Image> FromEncoded(const void* data, size_t size);
  static std::unique_ptr<Image> FromFile(const char* path);

  // From platform buffer (AHardwareBuffer / IOSurface / DMA-BUF fd)
  static std::unique_ptr<Image> FromBuffer(HardwareBuffer buffer);

  int width() const;
  int height() const;
};

}  // namespace native::ui
```

## Canvas (RAII Wrapper)

```cpp
namespace native::ui {

class Canvas {
public:
  explicit Canvas(Surface& surface);
  ~Canvas();  // auto restore

  // Primitive drawing
  void DrawRect(Rect rect, const Paint& paint);
  void DrawText(const std::string& text, Point pos, const Paint& paint);
  void DrawPath(const Path& path, const Paint& paint);

  // Image drawing (PNG, SVG, camera buffer, etc.)
  void DrawImage(const Image& image, Rect dest);
  void DrawImage(const Image& image, Rect src, Rect dest);

  // State management
  void ClipRect(Rect rect);
  void Translate(Point offset);
  void Save();
  void Restore();
};

}  // namespace native::ui
```

**Contract**: Save at entry, restore at exit. Nested save/restore pairs allowed.

## Paint

```cpp
namespace native::ui {

class Paint {
public:
  Paint& SetColor(Color color);
  Paint& SetAntiAlias(bool enabled);
  Paint& SetStrokeWidth(float width);
  Paint& SetStyle(PaintStyle style);
};

}  // namespace native::ui
```

**Contract**: All methods return `*this` for chaining. Defaults: black fill.

## Path

```cpp
namespace native::ui {

class Path {
public:
  Path& MoveTo(Point p);
  Path& LineTo(Point p);
  Path& CubicTo(Point c1, Point c2, Point end);
  Path& Close();
};

}  // namespace native::ui
```

## Skia Isolation Rules

- Only `render/` and `surface/` modules may depend on `@skia//:skia`
- No module outside these may `#include` any Skia header
- Surface, Image, Canvas, Paint, Path must not expose Skia types in their public signatures
- Enforced by CI: `bazel query 'somepath(//src/framework/..., @skia//:skia)'` must return paths only through render/ or surface/

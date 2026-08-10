# Render Interface Contract

**Last Updated**: 2026-07-29

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
│   │  DrawRect / DrawText / DrawPath / DrawImage│  │
│   │  Save / Restore / ClipRect / Translate    │  │
│   │  ~Canvas() → auto restore                 │  │
│   └───────────────────────────────────────────┘  │
│                                                   │
│  Surface::Flush() → commit pixels                 │
└─────────────────────────────────────────────────┘
```

**Lifecycle**:

```text
1. Create Surface          Surface::Create(w, h)  or  Surface::CreateFromBuffer(hardwareBuffer)
2. Attach Canvas           Canvas canvas(surface)
3. Draw                    canvas.DrawRect(...); canvas.DrawText(...); canvas.DrawImage(...)
4. Destroy Canvas          ~Canvas() → auto restore SkCanvas state
5. Flush                   surface.Flush()
6. Repeat from step 2 for next frame
```

**Contracts**:

| Role | Responsibility |
|------|---------------|
| `Surface` | Owns pixel buffer; create from size or external `HardwareBuffer`; `Flush()` to commit pixels |
| `Canvas` | Lightweight RAII attached to `Surface&`; all drawing APIs; auto save/restore on scope |

## Frame Pipeline: Change → Flush

When a single property changes (e.g. `State` → `RequestRedraw`), the system does NOT redraw everything — it tracks dirty widgets and only repaints what changed.

### Dirty Widget Tracking

Each widget carries two flags:

```cpp
class Widget {
  bool needs_layout_ = false;  // true → full re-measure + re-arrange + re-draw
  bool needs_draw_ = false;    // true → re-draw only (layout unchanged)
};
```

| Trigger | Flag Set | Next Frame Action |
|---------|----------|-------------------|
| `state->count = 42` (same size) | `needs_draw_ = true` | Draw only changed widgets |
| `AddChild(widget)` (structure change) | `needs_layout_ = true` | Full Measure → Arrange → Draw |
| `state->width = 200` (size change) | `needs_layout_ = true` | Full Measure → Arrange → Draw |

### Batch Coalescing

Multiple changes within one frame are coalesced — only one Flush at the end:

```text
Frame N:
  Worker:  state->count = 1   → mark dirty
  Worker:  state->count = 2   → still dirty (same widget)
  Worker:  state->name = "x"  → mark dirty
  Worker:  AddChild(new_btn)  → mark layout dirty
           ↓ (end of frame, batch flush)
  Main:    Measure() → Arrange() → Draw() → Surface::Flush()
```

### Full Sequence

```text
┌─────────────────────────────────────────────────────────────────────┐
│  1. Change triggers                                                │
│     state->count = 42                                               │
│     Property<int>::operator= → Signal() → RequestRedraw()           │
│         │                                                           │
│         ▼                                                           │
│  2. Mark dirty                                                      │
│     Widget::needs_draw_ = true                                      │
│     Propagate up to root for frame scheduling                       │
│         │                                                           │
│         ▼                                                           │
│  3. Frame loop begins (consumer-driven: vsync / timer / SwapBuffer) │
│         │                                                           │
│         ▼                                                           │
│  4. Batch state changes                                             │
│     Coalesce all pending Signals → single invalidation pass         │
│         │                                                           │
│         ▼                                                           │
│  5. If needs_layout_:                                               │
│       a. Container::Measure(available)  ← Yoga runs                 │
│       b. Container::Arrange(size)       ← positions updated         │
│         │                                                           │
│         ▼                                                           │
│  6. Render dirty widgets:                                           │
│       Canvas canvas(surface);                                       │
│       For each dirty widget (or all if layout changed):             │
│         canvas.Save()                                               │
│         canvas.Translate(widget.position)                           │
│         widget->Draw(canvas)                                        │
│         canvas.Restore()                                            │
│         │                                                           │
│         ▼                                                           │
│  7. canvas.~Canvas()  ← auto restore to entry save state           │
│         │                                                           │
│         ▼                                                           │
│  8. Surface::Flush()   ← commit pixels to display / buffer          │
│         │                                                           │
│         ▼                                                           │
│  9. Clear dirty flags, wait for next frame                          │
└─────────────────────────────────────────────────────────────────────┘
```

### Partial Draw Optimization

When only `needs_draw_` is set (no layout change), the frame loop can skip clean widgets:

```cpp
void RootWidget::Draw(Canvas& canvas) {
  for (auto& child : children_) {
    if (!child->needs_layout_ && !child->needs_draw_) {
      continue;  // skip — widget content unchanged
    }
    canvas.Save();
    canvas.Translate(child->position);
    child->Draw(canvas);
    canvas.Restore();
    child->needs_draw_ = false;   // clear flag
    child->needs_layout_ = false;
  }
}
```

For more aggressive optimization (dirty rect clipping), the root tracks a `dirty_rect_` that unions all changed regions, and passes it to `Canvas::ClipRect` before drawing.

## Canvas Sharing Across Widget Tree

The framework uses a **single Canvas per frame**, shared by all widgets. No widget creates its own Canvas or Surface.

### Architecture

```
Frame N:  Surface (pixel buffer, e.g. 800×600)
           │
           └── Canvas canvas(surface)    ← 唯一的 Canvas
                │
                ├── canvas.Save()
                ├── canvas.Translate(root.position)
                ├── Container::Draw(canvas)
                │    ├── canvas.Save()
                │    ├── canvas.Translate(child_A.position)
                │    ├── Text::Draw(canvas)            ← 同一 Canvas
                │    ├── canvas.Restore()    ← 回到 Container 坐标
                │    ├── canvas.Save()
                │    ├── canvas.Translate(child_B.position)
                │    ├── Button::Draw(canvas)          ← 同一 Canvas
                │    └── canvas.Restore()
                └── canvas.Restore()

          ~Canvas()  ← auto restore
          Surface::Flush()  ← 一次性提交
```

### Rules

| Rule | Explanation |
|------|-------------|
| One Surface per frame | Only one pixel buffer is active per rendering pass |
| One Canvas per frame | Canvas is created once, passed by reference along the widget tree |
| No per-widget Canvas | Widgets never create their own Canvas or Surface |
| Widget draws in local coords | Container translates the Canvas origin so each child draws relative to (0,0) = its top-left |
| Save/Restore isolates children | Each child's draw is wrapped in Save/Restore to prevent coordinate leakage |

### Why This Design

This matches how mainstream 2D UI frameworks work:

| Framework | Canvas Model |
|-----------|-------------|
| **Android View** | Single `Canvas` passed through `dispatchDraw()` — `save/translate/restore` per child |
| **Flutter** | Single `Canvas` from `PaintingContext` — `save/translate/restore` per child |
| **Our design** | Same — single Canvas, shared, coordinate transforms via Save/Translate/Restore |
| **React Native** | Different — each native View has its own backing store (multi-Surface) |

### Contrast: Multi-Surface (React Native Model)

```text
Our model (single Surface):
  Yoga → shared Canvas → save/translate/restore → Flush

RN model (per-View Surface):
  Yoga → each View renders independently → OS compositor blends → display
```

Our model is simpler (no compositor needed) and performant for Skia-rendered UIs.
The multi-Surface path is available via `Surface::CreateFromBuffer(HardwareBuffer)` for
platform video/camera overlays when needed.

## Surface (Backing Store)

```cpp
namespace native::ui {

// Rendering backend selection for buffer-backed surfaces/images.
// kCPU = raster (default); kGPU = Android-only GLES/EGL (requires a RenderContext).
enum class RenderBackend { kCPU, kGPU };

class Surface {
public:
  // Create a new rendering surface
  static std::unique_ptr<Surface> Create(int width, int height);

  // Create a surface from an external platform buffer.
  // kGPU requires a non-null RenderContext; falls back to kCPU, then nullptr.
  // Android-only: host builds are guarded stubs returning nullptr.
  static std::unique_ptr<Surface> CreateFromBuffer(HardwareBuffer buffer,
                                                   RenderBackend backend = RenderBackend::kCPU,
                                                   RenderContext* ctx = nullptr);

  ~Surface();

  // Present the rendered content (swap/flush for platform surfaces)
  void Flush();

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

  // From platform buffer (Android AHardwareBuffer; IOSurface / DMA-BUF reserved).
  // kGPU requires a non-null RenderContext (zero-copy texture import); kCPU copies.
  // Android-only: host builds are guarded stubs returning nullptr.
  static std::unique_ptr<Image> FromBuffer(HardwareBuffer buffer,
                                           RenderBackend backend = RenderBackend::kCPU,
                                           RenderContext* ctx = nullptr);

  int width() const;
  int height() const;
};

}  // namespace native::ui
```

## External Buffer Rendering (Android AHardwareBuffer)

Android-only feature (min API 29). **Android is the only implemented platform**; every
platform-specific implementation point is a guarded stub (`// TODO(android-only)` +
immediate return) on host builds, keeping host/CI green. GPU backend = **GLES/EGL**.

### HardwareBuffer (cross-platform wrapper)

```cpp
namespace native::ui {

class HardwareBuffer {
public:
  enum class Kind { kMemory, kAHardwareBuffer, kInvalid };

#if __ANDROID__
  static HardwareBuffer FromAHardwareBuffer(void* buffer);   // non-owning wrap
#endif
  static HardwareBuffer FromMemory(void* pixels, size_t row_bytes, int width, int height);

  bool IsValid() const;
  Kind kind() const;
  int width() const;      // AHardwareBuffer kind: lazily filled via AHardwareBuffer_describe
  int height() const;
  size_t row_bytes() const;  // stride in bytes; may exceed width*4 (row padding, FR-002)
  int format() const;        // 0 = unknown
  bool operator==(const HardwareBuffer&) const;  // compares the underlying handle

  const void* pixels() const;  // Memory kind only
};
}
```

Non-owning: the producer retains the buffer (FR-011); the framework copies what it renders.

### RenderBackend / RenderContext

- `RenderBackend::kCPU` (default): raster — `WrapPixels` over locked buffer memory (CPU
  `Surface`) or an owned copy (`Image`).
- `RenderBackend::kGPU`: zero-copy GLES/EGL — `GrAHardwareBufferUtils` backend-texture
  import. Requires a non-null `RenderContext`; falls back to kCPU otherwise.

```cpp
namespace native::ui {

// GPU/EGL context bundle; single-context rule: one EGLContext hosts both texture
// imports and the encoder-input-surface render target (zero-copy).
struct RenderContext {
  GrDirectContext* gr;
  void* display;  // EGLDisplay
  void* context;  // EGLContext
  void* surface;  // EGLSurface — from AMediaCodec_createInputSurface (Android)

  // __ANDROID__ only; nullptr on failure/host.
  static std::unique_ptr<RenderContext> CreateFromMediaCodecInputSurface(
      void* surface /* ANativeWindow* */, int width, int height);
  void MakeCurrent();
  void SwapBuffers();  // eglSwapBuffers -> presents to encoder
  ~RenderContext();
};
}
```

### AHwb (Android AHardwareBuffer utility layer)

`surface/ahwb.h/.cc` — `Describe`, `Lock`/`Unlock`, `Pixels` (RAII), `AllocateRgba`,
`WriteRgba` (stride-aware), `Release`, `ToCpuImage` (owned copy), `ToGpuImage`
(zero-copy), `DumpPng` (diagnostic). All `#if defined(__ANDROID__)`; host stubs return `-5`.

### ExternalImage widget

`widgets/external_image.h` — binds a `HardwareBuffer` and displays it. Rebuilds its image
only when the bound handle changes (`operator==` guard, FR-003/FR-007). `SetBuffer` /
`Watch(Property<HardwareBuffer>&)` drive live updates; default backend is `kCPU`.

### MediaCodec (encode egress, in scope)

Native NDK C API (`<media/NdkMediaCodec.h>`, `<media/NdkMediaMuxer.h>`) — no JNI.
`AMediaCodec_createInputSurface` → `ANativeWindow*` → `RenderContext::CreateFromMediaCodecInputSurface`
→ canvas on that surface → `SwapBuffers()` → encoder buffers → `AMediaMuxer` → MP4.
See `specs/011-ahwb-external-image/contracts/media-codec.md` for the full contract.

## Canvas (RAII Wrapper)

```cpp
namespace native::ui {

class Canvas {
public:
  // Attach to a Surface — Canvas renders into this backing store
  explicit Canvas(Surface& surface);
  ~Canvas();  // auto restore

  // Primitive drawing
  void DrawRect(Rect rect, const Paint& paint);
  void DrawText(const std::string& text, Point pos, const Paint& paint);
  void DrawPath(const Path& path, const Paint& paint);

  // Image drawing (PNG, SVG, camera buffer, etc.)
  void DrawImage(const Image& image, Rect dest);
  void DrawImage(const Image& image, Rect src, Rect dest);  // crop + scale

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
  Paint& SetAlpha(uint8_t alpha);
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
- `Surface`, `Image`, `Canvas`, `Paint`, `Path` must not expose Skia types in their public signatures
- Enforced by CI:

```bash
bazel query 'somepath(//src/framework/..., @skia//:skia)' \
  | grep -v '//src/framework/render' | grep -v '//src/framework/surface'
```

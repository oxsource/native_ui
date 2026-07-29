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
│  Surface::Present() → swap/flush                  │
└─────────────────────────────────────────────────┘
```

**Lifecycle**:

```text
1. Create Surface          Surface::Create(w, h)  or  Surface::CreateFromBuffer(hardwareBuffer)
2. Attach Canvas           Canvas canvas(surface)
3. Draw                    canvas.DrawRect(...); canvas.DrawText(...); canvas.DrawImage(...)
4. Destroy Canvas          ~Canvas() → auto restore SkCanvas state
5. Present                 surface.Present()
6. Repeat from step 2 for next frame
```

**Contracts**:

| Role | Responsibility |
|------|---------------|
| `Surface` | Owns pixel buffer; create from size or external `HardwareBuffer`; `Present()` to display |
| `Canvas` | Lightweight RAII attached to `Surface&`; all drawing APIs; auto save/restore on scope |

## Surface (Backing Store)

```cpp
namespace native::ui {

class Surface {
public:
  // Create a new rendering surface
  static std::unique_ptr<Surface> Create(int width, int height);

  // Create a surface from an external platform buffer
  static std::unique_ptr<Surface> CreateFromBuffer(HardwareBuffer handle);

  ~Surface();

  // Present the rendered content (swap/flush for platform surfaces)
  void Present();

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

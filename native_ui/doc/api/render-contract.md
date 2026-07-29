# Render Interface Contract

**Last Updated**: 2026-07-29

## Surface (Backing Store)

```cpp
namespace native::ui {

class Surface {
public:
  // Create a new rendering surface
  static std::unique_ptr<Surface> Create(int width, int height);

  // Create a surface from an external platform buffer
  static std::unique_ptr<Surface> CreateFromBuffer(BufferHandle handle);

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
  // Decode from encoded data (PNG, JPEG, WebP)
  static std::unique_ptr<Image> FromEncoded(const void* data, size_t size);
  static std::unique_ptr<Image> FromFile(const char* path);

  // From platform buffer (AHardwareBuffer / IOSurface / DMA-BUF fd)
  static std::unique_ptr<Image> FromBuffer(BufferHandle buffer);

  // From SVG text — rasterized at the given size
  static std::unique_ptr<Image> FromSvg(const char* xml, float width, float height);

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
  void DrawImage(BufferHandle buffer, Rect dest);           // convenience

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

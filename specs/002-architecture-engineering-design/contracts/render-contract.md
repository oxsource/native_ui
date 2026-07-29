# Render Interface Contract

**Purpose**: Define the Surface/Canvas/Paint/Path RAII wrappers and Skia isolation rules.

## Surface (Backing Store)

```cpp
namespace native::ui {

class Surface {
public:
  static std::unique_ptr<Surface> Create(int width, int height);
  static std::unique_ptr<Surface> CreateFromBuffer(BufferHandle handle);
  ~Surface();

  void Present();  // swap/flush for platform surfaces
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
  explicit Canvas(Surface& surface);  // attach to a backing store
  ~Canvas();  // auto restore

  void DrawRect(Rect rect, const Paint& paint);
  void DrawText(const std::string& text, Point pos, const Paint& paint);
  void DrawPath(const Path& path, const Paint& paint);
  void ClipRect(Rect rect);
  void Translate(Point offset);
  void Save();
  void Restore();
};

}  // namespace native::ui
```

**Contract**: Save at entry, restore at exit. Nested Save/Restore pairs allowed.
Canvas can be constructed with any Surface (display, offscreen, platform buffer).

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
- Surface, Canvas, Paint, Path must not expose Skia types in their public signatures
- Enforced by CI: `bazel query 'somepath(//src/framework/..., @skia//:skia)'` must return paths only through render/ or surface/

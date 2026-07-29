# Render Interface Contract

**Purpose**: Define the Canvas/Paint/Path RAII wrappers and Skia isolation rules.

## Canvas (RAII Wrapper)

```cpp
class Canvas {
public:
  explicit Canvas(SkCanvas* sk_canvas);
  ~Canvas();  // auto restore

  void DrawRect(Rect rect, const Paint& paint);
  void DrawSimpleText(const std::string& text, Point pos, const Paint& paint);
  void DrawPath(const Path& path, const Paint& paint);
  void ClipRect(Rect rect);
  void Translate(Point offset);
  void Save();
  void Restore();
};
```

**Contract**: Save at entry, restore at exit. Nested Save/Restore pairs allowed.

## Paint

```cpp
class Paint {
public:
  Paint& SetColor(Color color);
  Paint& SetAntiAlias(bool enabled);
  Paint& SetStrokeWidth(float width);
  Paint& SetStyle(PaintStyle style);
};
```

**Contract**: All methods return `*this` for chaining. Defaults: black fill.

## Path

```cpp
class Path {
public:
  Path& MoveTo(Point p);
  Path& LineTo(Point p);
  Path& CubicTo(Point c1, Point c2, Point end);
  Path& Close();
};
```

## Skia Isolation Rules

- Only `render/` and `surface/` modules may depend on `@skia//:skia`
- No module outside these may `#include` any Skia header
- Canvas, Paint, Path must not expose Skia types in their public signatures
- Enforced by CI: `bazel query 'somepath(//src/framework/..., @skia//:skia)'` must return paths only through render/ or surface/

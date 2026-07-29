# Render & Surface Interface Contract

## Surface

```cpp
namespace native::ui {

class Surface {
public:
  static std::unique_ptr<Surface> Create(int width, int height);
  static std::unique_ptr<Surface> CreateFromBuffer(HardwareBuffer buffer);
  ~Surface();

  void Flush();
  int width() const;
  int height() const;
};

}  // namespace native::ui
```

## HardwareBuffer

```cpp
namespace native::ui {

class HardwareBuffer {
public:
  // Platform-specific constructors
#if __APPLE__
  static HardwareBuffer FromIOSurface(void* iosurface);
#elif __linux__
  static HardwareBuffer FromDmaBuf(int fd);
#endif

  bool IsValid() const;
};

}  // namespace native::ui
```

## Canvas

```cpp
namespace native::ui {

class Canvas {
public:
  explicit Canvas(Surface& surface);
  ~Canvas();  // auto restore

  void DrawRect(Rect rect, const Paint& paint);
  void DrawText(const std::string& text, Point pos, const Paint& paint);
  void DrawPath(const Path& path, const Paint& paint);
  void DrawImage(const Image& image, Rect dest);
  void DrawImage(const Image& image, Rect src, Rect dest);

  void ClipRect(Rect rect);
  void Translate(Point offset);
  void Save();
  void Restore();
};

}  // namespace native::ui
```

## Paint (header-only)

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

## Image

```cpp
namespace native::ui {

class Image {
public:
  static std::unique_ptr<Image> FromEncoded(const void* data, size_t size);
  static std::unique_ptr<Image> FromFile(const char* path);
  static std::unique_ptr<Image> FromBuffer(HardwareBuffer buffer);

  int width() const;
  int height() const;
};

}  // namespace native::ui
```

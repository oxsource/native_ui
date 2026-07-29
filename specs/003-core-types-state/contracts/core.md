# Core Types Contract

## Rect

```cpp
struct Rect {
  float x, y, width, height;

  bool Contains(Point p) const;
  Rect Intersect(Rect other) const;
  Rect Union(Rect other) const;
  Rect Inset(EdgeInsets insets) const;
  Rect Offset(Point offset) const;
};
```

## Point

```cpp
struct Point {
  float x, y;

  Point operator+(Point other) const;
  Point operator-(Point other) const;
  float DistanceTo(Point other) const;
};
```

## Size

```cpp
struct Size {
  float width, height;

  bool IsEmpty() const;
};
```

## Color

```cpp
struct Color {
  uint8_t r, g, b, a;

  static constexpr Color kRed{255, 0, 0, 255};
  static constexpr Color kGreen{0, 255, 0, 255};
  static constexpr Color kBlue{0, 0, 255, 255};
  static constexpr Color kWhite{255, 255, 255, 255};
  static constexpr Color kBlack{0, 0, 0, 255};
  static constexpr Color kTransparent{0, 0, 0, 0};
};
```

## EdgeInsets

```cpp
struct EdgeInsets {
  float top, left, bottom, right;

  static EdgeInsets All(float v);
  static EdgeInsets Symmetric(float horizontal, float vertical);
  static EdgeInsets Only(float top, float right, float bottom, float left);
};
```

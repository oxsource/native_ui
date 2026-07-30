# Widget Property Tag Contracts

**Purpose**: Define all tagged parameter types for widget base properties, text properties, and image properties.

## Widget Base Tags (all widget types)

```cpp
namespace native::ui {

struct Width { float value; };
struct Height { float value; };
struct MinWidth { float value; };
struct MaxWidth { float value; };
struct Padding { EdgeInsets value; };
struct Background { Color value; };
struct BackgroundGradient { Gradient value; };
struct Enabled { bool value; };
struct Visible { bool value; };
struct Opacity { float value; };       // 0.0–1.0
struct CornerRadius { float value; };
struct BorderWidth { float value; };
struct BorderColor { Color value; };
struct ShadowOffset { Point value; };
struct ShadowRadius { float value; };
struct ShadowColor { Color value; };

}  // namespace native::ui
```

## Text Tags (Text, inherited by Button)

```cpp
namespace native::ui {

struct Content { std::string value; };
struct FontSize { float value; };
struct TextColor { Color value; };

enum class TextAlign {
  kLeft, kCenter, kRight,      // horizontal
  kTop, kBottom               // vertical (combined with horizontal)
};

struct FontFamily { std::string value; };
struct FontWeight { int value; };           // 100–900
struct LineHeight { float value; };          // multiplier (1.0 = single)
struct MaxLines { int value; };
enum class TextDecoration { kNone, kUnderline, kLineThrough };

}  // namespace native::ui
```

## Button Tags (Button only)

```cpp
namespace native::ui {

struct Label { std::string value; };        // alias for Content
struct OnClick { std::function<void()> value; };
struct NormalColor { Color value; };
struct PressedColor { Color value; };

}  // namespace native::ui
```

## Image Tags (ImageWidget)

```cpp
namespace native::ui {

struct ImageURI { std::string value; };
struct Placeholder { std::string value; };   // file path
struct ErrorImage { std::string value; };    // file path

enum class ScaleMode {
  kCenter,        // natural size, centered
  kCenterCrop,    // fill + crop uniformly
  kCenterInside,  // scale to fit entirely
  kFitStart,       // scale to fit, top-left aligned
  kFitEnd,         // scale to fit, bottom-right aligned
  kFillXY,         // stretch to fill (ignore aspect)
};

enum class Gravity {
  kTop, kBottom, kLeft, kRight, kCenter
};

}  // namespace native::ui
```

## ApplyStyle Interface

```cpp
namespace native::ui {

class Widget {
  // Apply a Style (merges at kInstance priority)
  void ApplyStyle(const Style& style);
};

}  // namespace native::ui
```

## Constructor Usage Examples

```cpp
// Widget base properties on any widget
Text(Content("Hi"), Width(200), Height(48), Background(kWhite),
     CornerRadius(8), Opacity(0.9), Padding({8, 4, 8, 4}));

// Text-specific properties
Text(Content("Hello"), FontSize(24), TextColor(kBlue),
     FontFamily("Helvetica"), FontWeight(700), TextAlign(TextAlign::kCenter),
     LineHeight(1.5), MaxLines(2));

// Button inherits Text + adds interactive
Button(Label("Submit"), Width(160), Height(48),
       Background(kBlue), TextColor(kWhite), CornerRadius(24),
       NormalColor(kBlue), PressedColor(kDarkBlue));

// Image with scale + async loading
ImageWidget(ImageURI("photo.png"), Width(200), Height(200),
            ScaleType(ScaleMode::kCenterCrop),
            ScaleGravity(Gravity::kTop),
            Placeholder("loading.png"), ErrorImage("broken.png"));

// Style-based
Style card;
card.setCornerRadius(12).setBackground(kWhite).setShadow(kShadowGray);
Container(Padding({16}), card, Children{...});
```

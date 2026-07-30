# Widget Style Tag Contracts

**Purpose**: Define all constructor tag types that delegate to `Style::setXxx()`. Tags are the user-facing API; Style is the unified storage.

## Widget Base Style Tags (all widget types)

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

## Text Style Tags (Text, inherited by Button)

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

## Button Style Tags (Button only)

```cpp
namespace native::ui {

struct Label { std::string value; };        // alias for Content
struct OnClick { std::function<void()> value; };
struct NormalColor { Color value; };
struct PressedColor { Color value; };

}  // namespace native::ui
```

## Image Style Tags (ImageWidget)

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

## Widget Style Integration

```cpp
namespace native::ui {

class Widget {
  Style style_;  // single storage for all visual/behavioral properties
public:
  const Style& style() const { return style_; }

  // Apply a Style — merges at kInstance priority, auto-calls RequestRedraw()
  void ApplyStyle(const Style& s);
};

// ProcessArg delegation pattern (all visual tags follow this):
// ProcessArg(Background tag)  →  style_.setBackground(tag.value)
// ProcessArg(FontSize tag)    →  style_.setFontSize(tag.value)
// ProcessArg(Width tag)       →  style_.setWidth(tag.value)
// ... 所有视觉标签都委托给 Style::setXxx()

// Draw implementation reads from style():
// void Text::Draw(Canvas& canvas) {
//   auto size = style().font_size();
//   auto color = style().text_color();
//   auto bg = style().background();
//   ...
// }
```



## Constructor Usage

### Tags that delegate to `Style::setXxx()` (Style Tags)

These go through `ProcessArg` → `Style::setXxx` → stored in `Widget::style_`:

```cpp
// Widget base style tags — available on ALL widget types
Width(200), Height(48), MinWidth(100), MaxWidth(400),
Padding({8,4,8,4}), Background(kWhite), BackgroundGradient(linearGrad),
Enabled(true), Visible(true), Opacity(0.9),
CornerRadius(8), BorderWidth(2), BorderColor(kGray),
ShadowOffset({0,2}), ShadowRadius(4), ShadowColor(kBlack),

// Text style tags — available on Text and Button (inherits Text)
FontSize(24), TextColor(kBlue), TextAlign(TextAlign::kCenter),
FontFamily("Helvetica"), FontWeight(700), LineHeight(1.5),
MaxLines(2), TextDecoration(TextDecoration::kUnderline),

// Button-style tags — Button only
NormalColor(kBlue), PressedColor(kDarkBlue),

// Image style tags — ImageWidget only
ScaleType(ScaleMode::kCenterCrop), ScaleGravity(Gravity::kTop),
Placeholder("loading.png"), ErrorImage("broken.png"),
```

### Tags that are NOT Style (widget-specific fields)

These are stored directly on the widget, not in `style_`:

```cpp
Content("Hi"), Label("Submit"),         // Text content
OnClick([]{}),                          // Button callback
ImageURI("photo.png"),                  // ImageWidget file path
Id("my_widget"),                         // Widget identifier
Direction(kRow), Gap(8), Margin(12),   // Container layout tags
```

### Combined Examples

```cpp
Text(Content("Hi"), FontSize(24), TextColor(kBlue), Background(kWhite));

Button(Label("Submit"), OnClick([&]{ ++count; }),
       Width(160), Height(48), Background(kBlue), TextColor(kWhite),
       NormalColor(kBlue), PressedColor(kDarkBlue), CornerRadius(24));

ImageWidget(ImageURI("photo.png"), Width(200), Height(200),
            ScaleType(ScaleMode::kCenterCrop));

// Style object — same as tags above, just via Style API
Style card;
card.setCornerRadius(12).setBackground(kWhite);
Container(Padding({16}), card, Children{...});
```

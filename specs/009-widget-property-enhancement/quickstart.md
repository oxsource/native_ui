# Developer Quickstart: Widget Property Enhancement

## Build & Test

```bash
bazel build //src/framework/widgets
bazel test //tests:widgets_test //tests:style_test //tests:glide_test
```

## Style Usage

```cpp
using namespace native::ui;

// 1. Set global default theme (once at startup)
Style theme;
theme.setFontSize(14).setTextColor(kDarkGray).setBackground(kWhite);
Style::SetDefault(theme);

// 2. Create reusable widget style
Style cardStyle;
cardStyle.setCornerRadius(8).setBackground(kWhite)
         .setShadow(kGray, {0, 2}, 4);

// 3. Apply via constructor tag — explicit tags override Style
Text(Content("Title"), cardStyle, FontSize(20), TextColor(kBlack));

// 4. Or apply at runtime
widget->ApplyStyle(cardStyle);
```

## Widget Properties

```cpp
// All widgets: Width, Height, Background, CornerRadius, Opacity, Border, Shadow, etc.
Container(Width(300), Padding({16}), Background(kLightGray), CornerRadius(12),
    Children{
        std::make_unique<Text>(Content("Hello"), FontSize(24), TextColor(kBlue)),
        std::make_unique<Button>(Label("Click"), NormalColor(kBlue),
                                 PressedColor(kDarkBlue), CornerRadius(24)),
    });
```

## Glide Async Loading

```cpp
// Init (once)
Glide::SetDefault(new DefaultGlide());

// ImageWidget
auto img = std::make_unique<ImageWidget>(
    ImageURI("photo.png"),
    Width(200), Height(200),
    ScaleType(ScaleMode::kCenterCrop),
    Placeholder("loading.png"),
    ErrorImage("broken.png"));
```

## Hello World (Beautified)

```cpp
auto theme = Style().setFontSize(16).setTextColor(kWhite);
auto card = Style().setCornerRadius(12).setBackground(kBlue)
                   .setShadow(Color{0,0,0,64}, Point{0,4}, 8);

Container(Width(320), Height(200), card,
    Padding({24}),
    Children{
        std::make_unique<Text>(Content("Count: 0"), theme, FontSize(28)),
        std::make_unique<Button>(Label("+1"), theme,
            Width(120), Height(44), CornerRadius(22),
            NormalColor(kWhite), TextColor(kBlue)),
    });
```

# Quickstart: Register & Use Fonts (External Font Paths)

**Feature**: `012-android-font-support`

## 1. Register a font by file path

```cpp
#include <native_ui/font.h>   // public header — FontManager + Font + constants

// Any font file reachable by the process (.ttf/.otf). Works identically on
// Android (API 29+), macOS, and Linux.
bool ok = native::ui::FontManager::Default().RegisterFont(
    "appfont", "/data/fonts/my-app.ttf");        // weight defaults to 400

if (!ok) {
  std::fprintf(stderr, "register failed: %s\n",
               native::ui::FontManager::Default().last_error().c_str());
}
```

Register multiple weight variants of the same family:

```cpp
namespace ui = native::ui;
auto& fm = ui::FontManager::Default();
fm.RegisterFont("appfont", "/data/fonts/my-app-regular.ttf", ui::FontManager::kFontWeightRegular);
fm.RegisterFont("appfont", "/data/fonts/my-app-bold.ttf",    ui::FontManager::kFontWeightBold);
```

`FontWeight(...)` picks the matching variant; when no exact weight is
registered, the nearest registered weight of that family is used (FR-004).

## 2. Use it on any Text/Button

```cpp
using namespace native::ui;

// Constructor tags:
Text(Content("Hello"), FontFamily("appfont"), FontWeight(kFontWeightBold),
     FontSize(kFontSizeHeadline));

// Applied Style (identical effect):
Style s;
s.setFontFamily("appfont").setFontWeight(kFontWeightRegular)
 .setPriority(StylePriority::kInstance);
textWidget->ApplyStyle(s);

// Sizing/weight shortcuts via the FontManager constants:
FontSize(FontManager::kFontSizeBody)      // 14
FontSize(FontManager::kFontSizeBodyLarge) // 16 (default)
FontWeight(FontManager::kFontWeightMedium) // 500
```

## 3. Default font (first registered, or explicit)

The **first successfully registered** font becomes the default, so text with no
`FontFamily` at all renders with it automatically:

```cpp
FontManager::Default().RegisterFont("first", "/data/fonts/base.ttf");
Text(Content("No family set → uses 'first'"));   // renders with base.ttf

FontManager::Default().RegisterFont("brand", "/data/fonts/brand.ttf");
FontManager::Default().SetDefaultFont("brand");  // explicit override (FR-014)
Text(Content("Now → uses 'brand'"));
```

Unregistered families and bad files fall back to the default font (never crash):

```cpp
Text(Content("Unknown family"), FontFamily("does_not_exist"));
// renders with the default font, no crash (FR-007)
```

## 4. Android device demo

```sh
make android-font-demo    # build + push + run; pulls rendered PNG via adb (see spec research Q6)
```

## Model

- Interface: `FontManager::Default()` singleton (like `Glide::Default()`).
- `Font{family, weight, size}` descriptor is passed to `Canvas::MeasureText` / `Canvas::DrawText`; widgets build it from `style()`.
- Detailed rules: `specs/012-android-font-support/contracts/font-manager.md`.
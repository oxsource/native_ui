# Style Contract

**Purpose**: Define Style class, StylePriority enum, and Merge algorithm for reusable property bundling.

## StylePriority

```cpp
namespace native::ui {

enum class StylePriority : int {
  kGlobal   = 100,
  kTheme    = 200,
  kClass    = 300,
  kInstance = 400,
  kExplicit = 500,
};

}  // namespace native::ui
```

## Style

```cpp
namespace native::ui {

class Style {
public:
  Style();  // initializes with kGlobal priority, all is_set = false

  // -- Priority --
  Style& setPriority(StylePriority p);
  StylePriority priority() const;

  // -- Common widget properties (chainable setters) --
  Style& setWidth(float);
  Style& setHeight(float);
  Style& setMinWidth(float);
  Style& setMaxWidth(float);
  Style& setPadding(EdgeInsets);
  Style& setBackground(Color);
  Style& setBackgroundGradient(Gradient);
  Style& setEnabled(bool);
  Style& setVisible(bool);
  Style& setOpacity(float);
  Style& setCornerRadius(float);
  Style& setBorderWidth(float);
  Style& setBorderColor(Color);
  Style& setShadowOffset(Point);
  Style& setShadowRadius(float);
  Style& setShadowColor(Color);

  // -- Text properties --
  Style& setFontSize(float);
  Style& setTextColor(Color);
  Style& setTextAlign(TextAlign);
  Style& setFontFamily(const std::string&);
  Style& setFontWeight(int);
  Style& setLineHeight(float);
  Style& setMaxLines(int);
  Style& setTextDecoration(TextDecoration);

  // -- Image properties --
  Style& setScaleType(ScaleMode);
  Style& setScaleGravity(Gravity);
  Style& setPlaceholder(const std::string& path);
  Style& setErrorImage(const std::string& path);

  // -- Global default --
  static void SetDefault(const Style& style);  // main-thread-only
  static const Style& Default();

private:
  friend Style Merge(const Style& base, const Style& overlay);

  // Per-property storage with is_set flags
  struct Data { /* ... all properties + is_set bools ... */ };
  Data data_;
  StylePriority priority_ = StylePriority::kGlobal;
};

// Merge two Styles: for each set property in overlay,
// if overlay.priority >= base.priority, overlay wins.
// Unset properties in overlay are ignored (base value survives).
Style Merge(const Style& base, const Style& overlay);

}  // namespace native::ui
```

## Widget Style Member

Each Widget stores all visual/behavioral properties in a single `Style style_` member — no duplicate fields.

```cpp
class Widget {
public:
  // Apply a Style — merges into style_ AND auto-calls RequestRedraw()
  void ApplyStyle(const Style& s);

  // Draw reads from style()
  const Style& style() const { return style_; }

protected:
  // ProcessArg delegates to style_.setXxx(...)
  void ProcessArg(Background tag) { style_.setBackground(tag.value); }

  Style style_;
};
```

- Tagged constructor params → `ProcessArg(Background tag)` → `style_.setBackground(value)`
- `ApplyStyle(style)` → `style_ = Merge(style_, style)` → `RequestRedraw()` (auto)
- `Widget::Draw()` reads visual props from `style()` — `style().background()`, `style().corner_radius()` etc.

## Usage

```cpp
// Global theme — all subsequent widgets inherit these defaults
Style theme;
theme.setFontSize(16).setTextColor(kBlack).setBackground(kWhite);
Style::SetDefault(theme);  // kGlobal priority

// Widget-specific Style
Style cardStyle;
cardStyle.setPriority(StylePriority::kInstance);
cardStyle.setCornerRadius(8).setBackground(kLightGray);

// ApplyStyle merges + auto-RequestRedraw
widget->ApplyStyle(cardStyle);

// Tags still work — they delegate to Style::setXxx at construction
Text(Content("Title"), FontSize(24), TextColor(kDarkBlue));
```

## Merge Rules

1. For each property in `overlay`: if `overlay.is_set && overlay.priority >= base.priority`, overlay's value replaces base's value
2. Properties not set in `overlay` are ignored — base value survives
3. If `overlay.priority == base.priority`, overlay wins (last-write semantics)
4. Global default priority chain: `SetDefault(kGlobal) → Widget ctor(kClass) → ApplyStyle(kInstance) → Explicit(kExplicit)`

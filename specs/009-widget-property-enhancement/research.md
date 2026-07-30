# Research: Widget Property Enhancement

**Date**: 2026-07-30

## Decisions

### Style: Value Type with Per-Property is_set + Priority

- **Decision**: `Style` is a plain data class. Each property has an `is_set` flag and is stored by value. `StylePriority` enum (kGlobal=100→kExplicit=500) controls merge ordering. `Merge(base, overlay)` compares per-property: `overlay.is_set && overlay.priority >= base.priority → overlay wins`.
- **Rationale**: Value semantics avoid shared-state bugs. `is_set` enables partial styles — unset properties fall through to lower priority levels. Priority-based merge allows global themes that widgets can selectively override.
- **Alternatives considered**: Shared pointer/COW (rejected: unnecessary complexity for small data); Inheritance-based hierarchy (rejected: too rigid); JSON-like dynamic properties (rejected: type safety loss)

### Widget Properties: Inline Storage, Not External Attributes

- **Decision**: All widget base properties (Width, Height, Background, etc.) are stored as member fields on `Widget` base class. Text properties on `Text`. ApplyStyle copies Style values into these fields.
- **Rationale**: Direct member access is zero-overhead at draw time. No map lookup or dynamic dispatch. Properties are visible in the debugger as plain fields.
- **Alternatives considered**: Separate attributes map (rejected: runtime overhead); External property list (rejected: breaks OOP); CRTP mixin (rejected: template bloat)

### Button Inherits Text

- **Decision**: `class Button : public Text`. Button inherits all Text properties (FontSize, TextColor, etc.) plus adds NormalColor/PressedColor. Button::Draw calls Text::Draw for label, then applies state color overlay.
- **Rationale**: Eliminates property duplication. Any property added to Text is automatically available on Button. Matches Android (Button extends TextView) and Flutter (ElevatedButton uses Text).
- **Alternatives considered**: Button as standalone Widget with duplicated properties (rejected: maintenance burden); Composition (Text* member, rejected: awkward API)

### Glide: Global Singleton with Default Implementation

- **Decision**: `Glide` abstract base class with `Load/Cancel/ClearCache`. `DefaultGlide` implements LRU-cached async decoding via `std::async`. `Glide::SetDefault()` registers the global instance (main-thread-only). `ImageURI` tag triggers `Glide::Default()->Load()`.
- **Rationale**: Singleton is simple and matches Android Glide's `Glide.with()`. Abstract base allows custom implementations (e.g., network loader). DefaultGlide uses std::async instead of a custom thread pool for simplicity.
- **Alternatives considered**: Per-widget ImageLoader (rejected: no cache sharing); std::thread pool (rejected: std::async is sufficient for file I/O); Custom thread pool (overkill for file decode)

### Gradient: Separate Type, Applied as Background Shader

- **Decision**: `Gradient` is a standalone type with `Linear(from, to, stops)` and `Radial(center, radius, stops)` static factories. `BackgroundGradient(gradient)` tag on Widget base applies it. Canvas draws gradient shader via Skia `SkGradientShader`.
- **Rationale**: Separating Gradient from Color avoids combinatorial complexity. A separate tag (`BackgroundGradient`) coexists with `Background` — Gradient wins when both set.
- **Alternatives considered**: Gradient as part of Color union (rejected: complexity); Background as variant<Color, Gradient> (rejected: template overhead)

### ScaleType: Android ImageView Model

- **Decision**: `ScaleType(ScaleMode)` with modes: kCenter, kCenterCrop, kCenterInside, kFitStart, kFitEnd, kFillXY. `ScaleGravity(Gravity)` (kTop/kCenter/kBottom/kLeft/kRight) controls crop anchor. Default is kCenterCrop + kCenter.
- **Rationale**: Android's ScaleType is extensively validated and familiar to mobile developers. Gravity control for crop position is essential for face/feature-aware cropping.
- **Alternatives considered**: Simplified 3-mode (rejected: insufficient for photo UIs); Custom transform matrix (rejected: too low-level)

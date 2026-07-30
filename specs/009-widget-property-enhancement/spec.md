# Feature Specification: Widget Property Enhancement

**Feature Branch**: `009-widget-property-enhancement`

**Created**: 2026-07-30

**Status**: Draft

**Input**: User description: "对基础控件的属性进行丰富完善"

## Clarifications

### Session 2026-07-30

- Q: Text 属性设计参考方向 → A: 参考 Android TextView 设计——Text 拥有 TextColor(文字颜色)、TextAlign(对齐)、FontSize(字体大小)、FontFamily、LineHeight、MaxLines 等自有属性
- Q: 背景色归属范围 → A: 背景色归属于 Widget 基类，所有控件（Text、Button、Image、Container、Stack 等）都必须支持设置背景色
- Q: Button 与 Text 的继承关系 → A: Button 继承自 Text，获得 Text 全部属性（Content、FontSize、TextColor、TextAlign 等），在此基础上增加 OnClick、NormalColor、PressedColor、Disabled 等交互属性
- Q: 需要 Style 复用机制 → A: 设计 Style 对象，支持链式设置（`Style().setFontSize(16).setTextColor(kRed)`），可通过构造函数标签 `Text(Content("Hi"), myStyle)` 或运行时 `widget->ApplyStyle(myStyle)` 应用
- Q: Widget 基础属性范围 + Width/Height 语义 → A: Widget 基类应有 Width, Height, Background, Enabled, Visible, Opacity, CornerRadius, BorderWidth, BorderColor 等基础属性；Width/Height 为 CSS 语义的首选尺寸，作为 Yoga 布局约束而非强制固定
- Q: Style 优先级 + 默认值机制 → A: Style 对象内部携带 `StylePriority` 枚举值（kGlobal=100 → kTheme=200 → kClass=300 → kInstance=400 → kExplicit=500），合并两个 Style 时按优先级裁决：同属性取优先级高者，同优先级取后者。每个属性有独立 is_set 标志，未设置的属性不参与合并。`Style::SetDefault` 设 kGlobal 级别，Widget 构造时设 kClass 级别，ApplyStyle 设 kInstance 级别，显式标签设 kExplicit 级别
- Q: 补充缺失属性（Padding/Shadow/MinMax/Gradient）→ A: 新增 Padding(EdgeInsets)、MinWidth(float)/MaxWidth(float)、Shadow(offset,radius,color)、BackgroundGradient(Gradient) 属性——Padding 为 Widget 基类属性（CSS 语义内边距），MinWidth/MaxWidth 通过 Yoga 约束实现，Shadow 使用 Skia 阴影绘制，Gradient 支持线性/径向渐变
- Q: Image 缩放/裁剪模式 → A: 参考 Android ImageView ScaleType，Image 控件新增 `ScaleType(ScaleMode)` 和 `CropGravity(Gravity)` 属性。ScaleMode 枚举：kCenter（不缩放）、kCenterCrop（等比填充+裁剪）、kCenterInside（等比缩放到完全可见）、kFitEnd/FitStart（对齐边界的等比缩放）、kFillXY（拉伸填满）；CropGravity 控制裁剪对齐位置（kTop/kCenter/kBottom/kLeft/kRight）
- Q: Image 异步加载方案 → A: 参考 Glide 设计轻量加载器 `Glide` 全局单例，`Glide::Load()` 异步解码本地文件，`Glide::Cancel()` 取消请求；ImageWidget 通过 `ImageURI(path)` 标签触发 `Load()`，自动管理生命周期；内置 `DefaultGlide` 实现含 LRU 内存缓存 + 线程池

## User Scenarios & Testing

### User Story 1 - Developer Configures Visual Properties via Style (Priority: P1)

A developer defines a reusable `Style` object bundling common widget properties (width, height, background, enabled, opacity, corner radius) and typographic properties (font size, text color, alignment) — and applies it to multiple Text and Button widgets. The Style reduces duplication and ensures consistent appearance.

**Why this priority**: Style is the core reuse mechanism. Without it, every widget must repeat the same property tags, making UIs verbose and inconsistent.

**Independent Test**: A developer creates a `Style` with FontSize(16) and TextColor(kBlue), applies it to two different Text widgets, and verifies both render with the same font size and color.

**Acceptance Scenarios**:

1. **Given** a Style with `FontSize(18)` and `TextColor(kRed)`, **When** applied to a Text widget, **Then** the widget renders text at size 18 in red
2. **Given** a Style applied to a Button (which inherits from Text), **When** the Button is drawn, **Then** the Button label uses the Style's font and color
3. **Given** a Style applied at construction (`Text(Content("Hi"), myStyle)`), **When** drawn, **Then** the Style properties take effect
4. **Given** a widget with both explicit tags and a Style, **When** drawn, **Then** explicit tags override Style properties

---

### User Story 2 - Developer Customizes Text Appearance (Priority: P1)

A developer configures Text and Button appearance using Android-inspired properties: background color, text color, font size, alignment, font family, line height, and overflow behavior.

**Why this priority**: Text is the root of Button (Button inherits Text). Rich text/styling properties on Text benefit both labels and buttons, covering the majority of UI customization needs.

**Independent Test**: A developer creates a Text widget with `Background(kWhite)`, `TextColor(kBlue)`, `TextAlign(kCenter)`, `FontSize(24)`, renders it, and verifies the pixel output matches each property.

**Acceptance Scenarios**:

1. **Given** a Text widget with `Background(kWhite)` and `TextColor(kBlue)`, **When** drawn, **Then** the background is a white rect and the text is blue
2. **Given** a Text widget with `TextAlign(kCenter)`, **When** drawn in a wide container, **Then** the text is horizontally centered within the widget bounds
3. **Given** a Text widget with `FontSize(24)` and `FontFamily("Helvetica")`, **When** drawn, **Then** the text renders at 24px in Helvetica
4. **Given** a Text widget with `LineHeight(1.5)` and `MaxLines(2)` with overflow content, **When** drawn, **Then** line spacing is 1.5x and the second line is truncated with an ellipsis

---

### User Story 3 - Developer Creates Styled Buttons Inheriting Text Properties (Priority: P2)

A developer creates a Button that inherits all Text properties (font size, text color, background, alignment) plus adds interactive state colors for normal, pressed, and disabled states. Since Button extends Text, the same Style object works for both.

**Why this priority**: Button inheriting Text eliminates property duplication. Text properties like FontSize, TextColor, and Background are automatically available on Button, and a single Style can drive both labels and buttons.

**Independent Test**: A developer creates a Style with FontSize(16) and TextColor(kWhite), applies it to a Button with Background(kBlue), triggers a press, and verifies the button renders in blue with white 16px text, then darkens when pressed.

**Acceptance Scenarios**:

1. **Given** a Button with `Background(kBlue)`, `TextColor(kWhite)`, and `PressedColor(kDarkBlue)`, **When** a MouseEvent is pushed at the button position, **Then** the button renders in dark blue while pressed (inheriting Text properties for the label rendering)
2. **Given** a Button with `Disabled(true)`, **When** drawn, **Then** the button renders with reduced opacity and does NOT respond to click events
3. **Given** a Button with `HoverColor(kLightGray)`, **When** the cursor enters the button area, **Then** the button renders in light gray

---

### User Story 4 - Developer Controls Image Scaling and Cropping (Priority: P2)

A developer configures how an Image widget scales and crops its content within the widget bounds using Android ImageView-inspired ScaleType modes: center, center-crop, center-inside, fit-start/end, and fill.

**Why this priority**: Different image types need different scale behaviors — avatars use center-crop, product photos use center-inside, banners use fill. Missing modes force developers to manually calculate transforms.

**Independent Test**: A developer creates two Image widgets with different ScaleType + ScaleGravity combinations (kCenterCrop+kTop vs kCenterCrop+kCenter), renders them with the same image, and verifies the visible regions differ.

**Acceptance Scenarios**:

1. **Given** an Image widget with `ScaleType(kCenterCrop)`, **When** drawn with different aspect ratio, **Then** the image fills the bounds uniformly, cropping excess, with center gravity by default
2. **Given** an Image widget with `ScaleType(kCenterInside)`, **When** drawn with different aspect ratio, **Then** the image is scaled to fit entirely within bounds while preserving aspect ratio
3. **Given** an Image widget with `ScaleType(kCenter)`, **When** drawn, **Then** the image is rendered at its natural size, centered in widget bounds, no scaling
4. **Given** an Image widget with `ScaleType(kCenterCrop)` and `ScaleGravity(kTop)`, **When** drawn, **Then** the visible crop region is anchored to the top of the image
5. **Given** an Image widget with `ScaleType(kFillXY)`, **When** drawn with different aspect ratio, **Then** the image stretches to fill bounds exactly (aspect ratio not preserved)

---

### Edge Cases

- What happens when Opacity is set to 0 (invisible but still occupies layout space)?
- What happens when both Opacity and Visible(false) are set?
- What happens when CornerRadius is larger than half the widget size?
- What happens when FontFamily specifies a font that is not installed?
- What happens when MaxLines is 0 or negative?
- What happens when an image FitMode is applied to a zero-sized widget?
- What happens when a Button is both Disabled and pressed?
- What happens when a Style and an explicit tag specify the same property (e.g., FontSize in both)?
- What happens when `widget->ApplyStyle(style)` is called after construction — does it override or merge?
- What happens when a Button inheriting Text has both `Content` (from Text) and `Label` (from Button) set?
- What happens when Width(0) or Height(0) is set?
- What happens when Width and Height are set via Style and then overridden by parent FlexLayout?
- What happens when Enabled(false) widget receives a mouse event?
- What happens when Width/Height is set but layout_size_ is also set via Container::Layout(Size)?
- What happens when both Background(Color) and BackgroundGradient(gradient) are set — which wins?
- What happens when Padding exceeds container size?
- What happens when ShadowRadius is 0?
- What happens when MinWidth > MaxWidth?
- What happens when a linear gradient has zero-length from→to vector?
- What happens when Image has no source set (null image) and Placeholder is set?
- What happens when Glide::Default() is null (not initialized)?
- What happens when Placeholder file path is invalid?
- What happens when ImageURI changes before the previous Load completes?
- What happens when ScaleType(kCenter) is used and the image is larger than widget bounds?
- What happens when ScaleType(kCenterInside) is used and the image is smaller than widget bounds — is it upscaled?
- What happens when ScaleGravity is set but ScaleType doesn't support gravity (e.g., kFillXY)?
- What happens when Image has no source set (null image)?

## Requirements

### Functional Requirements

- **FR-001**: A `Style` class MUST support chainable property setting: `Style().setFontSize(16).setTextColor(kRed).setWidth(200).setHeight(48)` — all properties are optional
- **FR-002**: `Style` MUST cover at minimum: Width, Height, MinWidth, MaxWidth, Padding, Background, BackgroundGradient, Enabled, Opacity, CornerRadius, BorderWidth, BorderColor, ShadowOffset, ShadowRadius, ShadowColor, FontSize, TextColor, TextAlign, FontFamily, FontWeight, LineHeight, MaxLines, ScaleType, ScaleGravity, Placeholder, ErrorImage
- **FR-003**: Style MUST carry a `StylePriority` enum value (kGlobal=100, kTheme=200, kClass=300, kInstance=400, kExplicit=500) — each property inherits the Style's priority
- **FR-004**: A free function `Style Merge(const Style& base, const Style& overlay)` MUST merge two Styles per-property: for each property where `overlay.is_set`, if `overlay.priority >= base.priority`, overlay wins; unset properties are ignored
- **FR-005**: Style MUST support `Style::SetDefault(const Style&)` — sets a global default (kGlobal priority) applied to all subsequently created widgets (main-thread-only)
- **FR-006**: Style MUST track per-property `is_set` flags — an unset property does NOT participate in Merge, allowing lower-priority values to survive
- **FR-007**: Widget base MUST support `Width(float)` and `Height(float)` — CSS 语义的首选尺寸，作为 Yoga 布局约束，可能被父容器拉伸
- **FR-008**: Widget base MUST support `Enabled(bool)` — when false, widget does not respond to events and renders with visual dimming; true by default
- **FR-009**: Widget base MUST support `Padding(EdgeInsets)` — CSS 语义内边距，通过 Yoga padding 约束实现，影响子控件布局位置
- **FR-010**: Widget base MUST support `MinWidth(float)` and `MaxWidth(float)` — CSS clamp 语义，通过 Yoga 约束实现，限制控件最小/最大宽度
- **FR-011**: Widget base MUST support `Background(Color)`, `BackgroundGradient(Gradient)`, `Opacity(float)`, `CornerRadius(float)`, `BorderWidth(float)`, `BorderColor(Color)`, `Visible(bool)` — common visual properties shared by all widget types
- **FR-010**: Text widget MUST own the following tagged properties: `Content(string)`, `FontSize(float)`, `TextColor(Color)`, `TextAlign(TextAlign)`, `FontFamily(string)`, `FontWeight(int)`, `LineHeight(float)`, `MaxLines(int)`, `TextDecoration(TextDecoration)`
- **FR-011**: Text widget MUST support `TextAlign(TextAlign)` — kLeft, kCenter, kRight (horizontal), and `kTop`, kCenter, kBottom (vertical)
- **FR-012**: Widget base MUST support `ShadowOffset(Point)`, `ShadowRadius(float)`, `ShadowColor(Color)` — 控件阴影，通过 Skia 阴影 API 绘制在背景层下方
- **FR-013**: Button MUST inherit from `Text` — all Text properties (Content, FontSize, TextColor, etc.) are automatically available on Button
- **FR-014**: Button MUST support additional tagged properties: `Label(string)` (wraps Content), `OnClick(function)`, `NormalColor(Color)`, `PressedColor(Color)`; Button also inherits `Enabled(bool)` from Widget base
- **FR-015**: Image widget MUST support `ScaleType(ScaleMode)` — kCenter (不缩放居中), kCenterCrop (等比填充裁剪), kCenterInside (等比缩放到完全可见), kFitStart (等比缩放＋左上对齐), kFitEnd (等比缩放＋右下对齐), kFillXY (拉伸填满)
- **FR-016**: Image widget MUST support `ScaleGravity(Gravity)` — 控制缩放后图像在 widget bounds 内的对齐/裁剪位置：kTop, kBottom, kLeft, kRight, kCenter; 与 ScaleType 组合使用（如 kCenterCrop + kTop 表示从顶部开始裁剪）
- **FR-017**: `Glide` class MUST provide global singleton with `Load(path, callback, options)` and `Cancel(request_id)` — 异步加载本地图片文件，不阻塞主线程
- **FR-018**: `Glide::Default()` MUST return the current instance; `Glide::SetDefault()` sets it (main-thread-only)
- **FR-019**: ImageWidget MUST support `ImageURI(string)` tag — 触发 Glide 异步加载，从 `Glide::Default()` 发起请求
- **FR-020**: ImageWidget MUST support `Placeholder(string)` and `ErrorImage(string)` tags — 加载中显示占位图，失败显示错误图
- **FR-021**: ImageWidget MUST cancel pending Glide request on destruction and on URI change
- **FR-022**: Image widget MUST support `CornerRadius(float)` for rounded corners
- **FR-023**: All properties MUST be configurable via tagged parameters in the constructor AND via Style
- **FR-024**: `Gradient` type MUST support `Linear(Point from, Point to, std::vector<ColorStop>)` and `Radial(Point center, float radius, std::vector<ColorStop>)` — 通过 Skia `SkGradientShader` 实现
- **FR-025**: Properties MUST NOT break existing widget API — all existing constructors remain valid

### Key Entities

- **Style**: A reusable bundle of visual and typographic properties. Carries a `StylePriority` enum value (kGlobal/kTheme/kClass/kInstance/kExplicit). Each property has an `is_set` flag. Two Styles merge via `Merge(base, overlay)`: for each set property in overlay, if `overlay.priority >= base.priority`, overlay's value wins. Supports chainable setters and `Style::SetDefault()`.
- **Text**: Inherits Widget base properties (Width, Height, Background, etc.) plus owns typographic properties (FontSize, TextColor, TextAlign, FontFamily, FontWeight, LineHeight, MaxLines, TextDecoration).
- **Button**: Inherits from Text — gets all Text + Widget properties automatically. Adds interactive properties: OnClick, NormalColor, PressedColor. Inherits Enabled from Widget base (Disabled behavior unified across all widgets). Button's Draw uses state color for background and Text properties for label rendering.
- **Image Widget**: Displays images with Android ImageView-inspired scale/crop control. `ScaleType` selects scale algorithm. `ScaleGravity` selects crop/anchor position. `ImageURI` triggers async loading via `Glide`. `Placeholder` and `ErrorImage` show loading/error states. Supports CornerRadius for rounded corners.
- **Glide**: Global singleton for asynchronous image loading. `Glide::Load(path, callback, options)` decodes images on a worker thread, returns results via callback on main thread. `Glide::Cancel(request_id)` aborts pending requests. Built-in `DefaultGlide` implementation provides LRU memory cache and thread pool. Inspired by Android Glide.
- **Widget Base Properties**: Width, Height, MinWidth, MaxWidth, Padding, Background, BackgroundGradient, ShadowOffset, ShadowRadius, ShadowColor, Enabled, Visible, Opacity, CornerRadius, BorderWidth, BorderColor — ALL widget types share these common visual/behavioral/layout properties
- **Gradient**: Describes a linear or radial color gradient. `Linear(from, to, colorStops)` for linear gradients; `Radial(center, radius, colorStops)` for radial. Each ColorStop is `(position, color)`. Rendered via Skia gradient shader.

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can set `Width(200)` and `Height(48)` on a Container and verify the layout respects the preferred size — testable via Measure output
- **SC-002**: A developer can set `Enabled(false)` on a Button and verify that a mouse event is NOT delivered — testable via callback flag
- **SC-003**: A developer can set Opacity(0.5) on any widget and verify the output pixels have 50% reduced alpha — testable via pixel readback
- **SC-004**: A developer can set Background + CornerRadius on any Container and verify rounded rect rendering — testable via pixel readback
- **SC-005**: Text with FontWeight(700) renders visibly thicker glyphs than FontWeight(400) — testable via pixel comparison
- **SC-006**: Image with ScaleType(kCenterCrop) fills the entire widget bounds, cropping uniformly — testable via edge pixel verification
- **SC-007**: Image with ScaleType(kCenterCrop) + ScaleGravity(kTop) crops from the top edge — different edge pixels compared to kCenter gravity
- **SC-008**: An ImageWidget with ImageURI triggers Glide::Load() — verify request_id is non-zero
- **SC-009**: An ImageWidget destroyed before Glide callback fires does NOT invoke the callback — verified by loaded flag never set
- **SC-010**: All new properties are settable via tagged parameters and do not break existing construction patterns — verified by existing test suite
- **SC-011**: A Text widget with MaxLines(1) and overflow truncates content with an ellipsis — testable via pixel readback
- **SC-010**: A developer can set Padding(8) on a Container and verify children are inset by 8px — testable via layout result positions
- **SC-011**: A developer can set MinWidth(100) and MaxWidth(400) on a Container and verify Yoga respects the clamp — testable via Measure output
- **SC-012**: A developer can set Shadow on a Container and verify a shadow is rendered below the background — testable via pixel readback
- **SC-013**: A developer can set BackgroundGradient(Linear(...)) on a Container and verify smooth color transition — testable via pixel sampling at gradient endpoints

## Assumptions

- Style is a plain data class with per-property `is_set` flags, a `StylePriority` value, and value semantics
- Style::SetDefault() sets kGlobal priority, main-thread-only, affects only widgets created AFTER the call
- Widget constructor creates Style at kClass priority and merges with global default: `ApplyStyle(Merge(Style::Default(), classStyle))`
- ApplyStyle(style) calls `Merge(widgetStyle, style)` with the incoming style's priority
- Explicit constructor tags set properties at kExplicit priority — the highest level, always wins
- Merge function compares integer priority values: higher wins; equal priority = overlay wins (last wins)
- Button inherits from Text: `class Button : public Text` — Button's Draw first draws Text (label + styling), then applies state color overlay
- Text alignment uses Skia's `SkFont` horizontal alignment; vertical alignment adjusts text Y position within bounds
- Background color on ANY widget draws a filled rect before content — Canvas draws the background rect, then clips, then calls the widget's Draw
- Width/Height are preferred sizes (CSS semantics) — set via `YGNodeStyleSetWidth/Height` as constraints, parent layout can override via FlexGrow
- Enabled(false) disables event delivery AND applies visual dimming (0.5 opacity overlay) — unified across all widgets
- Button's `Disabled` concept is unified into Widget base `Enabled` — `Enabled(false)` replaces `Disabled(true)` on Button
- Opacity is implemented as a canvas alpha multiplier via `Canvas::SaveLayer` or paint alpha
- CornerRadius uses Skia's `SkRRect` for drawing rounded rects
- Font properties use Skia's `SkFont` — TextLayout (SkParagraph) is still deferred post-MVP
- Button state defaults: NormalColor=light gray, PressedColor=darker gray; Enabled(true) by default
- ScaleType is implemented via `Canvas::DrawImage` with source/dest rect transformations based on image natural size and widget bounds
- ScaleGravity defaults to kCenter when not set; ignored by ScaleType(kFillXY) which fills regardless
- kCenterInside does NOT upscale images smaller than widget bounds — image stays at natural size, centered
- kCenter renders image at natural size, centered, no scaling — if larger than bounds, edges are clipped
- Glide::Load() decodes on a worker thread via `std::async` — callback is invoked on the main thread
- DefaultGlide uses LRU memory cache (50MB default) with `std::list` + `unordered_map` implementation
- DefaultGlide thread pool size defaults to 2 — sufficient for local file decoding
- ImageWidget calls `Load()` on construction if ImageURI is provided, and cancels on destruction/URI change
- Glide callback checks a per-widget `load_key_` to discard stale callbacks (widget reused or URI changed)
- Visible(false) skips Draw but does NOT remove the widget from layout
- Label("OK") on Button is synonymous with Content("OK") for convenience
- Padding is implemented as Yoga padding (`YGNodeStyleSetPadding`) — affects child layout positions inside Container/Stack
- MinWidth/MaxWidth are implemented as Yoga min/max constraints — limit widget sizing
- Shadow is rendered via Skia shadow API — drawn beneath background, before content
- Gradient uses Skia `SkGradientShader::MakeLinear/MakeRadial` — applied as background fill shader
- When both Background(Color) and BackgroundGradient(Gradient) are set, Gradient wins

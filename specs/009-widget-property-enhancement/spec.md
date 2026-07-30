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
- Q: Style 优先级 + 默认值机制 → A: 四级优先级：① 全局默认样式 (Style::SetDefault) ② 控件类硬编码默认值 ③ 实例 Style (构造参数/ApplyStyle) ④ 显式标签参数（最高）；高优先级覆盖低优先级，未设置的属性透传至更低级；Style 使用值拷贝，每个属性有独立 is_set 标志；Style::SetDefault 仅影响后续创建的控件，线程安全限制为 main-thread-only

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

### User Story 4 - Developer Adds Image Fit Modes (Priority: P2)

A developer configures how an Image widget scales its content within the widget bounds using fit modes: fill, contain, cover, or stretch.

**Why this priority**: Image fit modes are essential for displaying photos and icons correctly without distortion or cropping.

**Independent Test**: A developer creates two Image widgets with different FitMode values (kContain vs kCover), renders them with the same image and bounds, and verifies the output pixel content differs.

**Acceptance Scenarios**:

1. **Given** an Image widget with `FitMode(kContain)`, **When** drawn with aspect ratio different from bounds, **Then** the image is scaled to fit within bounds preserving aspect ratio
2. **Given** an Image widget with `FitMode(kCover)`, **When** drawn, **Then** the image fills the bounds, cropping excess
3. **Given** an Image widget with `FitMode(kFill)`, **When** drawn, **Then** the image stretches to fill bounds exactly

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

## Requirements

### Functional Requirements

- **FR-001**: A `Style` class MUST support chainable property setting: `Style().setFontSize(16).setTextColor(kRed).setWidth(200).setHeight(48)` — all properties are optional
- **FR-002**: `Style` MUST cover at minimum: Width, Height, Background, Enabled, Opacity, CornerRadius, BorderWidth, BorderColor, FontSize, TextColor, TextAlign, FontFamily, FontWeight, LineHeight, MaxLines
- **FR-003**: Style MUST support `Style::SetDefault(const Style&)` — sets a global default theme applied to all subsequently created widgets (main-thread-only)
- **FR-004**: Style MUST track per-property `is_set` flags — an unset property falls through to the next priority level without overriding
- **FR-005**: Style priority hierarchy MUST be (low→high): Global default → Widget class default → Instance Style (via ApplyStyle or constructor) → Explicit constructor tags
- **FR-006**: Style MUST be copyable via value copy (no shared pointers) — each widget holds its own Style copy after ApplyStyle
- **FR-007**: Widget base MUST support `Width(float)` and `Height(float)` — CSS 语义的首选尺寸，作为 Yoga 布局约束，可能被父容器拉伸
- **FR-008**: Widget base MUST support `Enabled(bool)` — when false, widget does not respond to events and renders with visual dimming; true by default
- **FR-009**: Widget base MUST support `Background(Color)`, `Opacity(float)`, `CornerRadius(float)`, `BorderWidth(float)`, `BorderColor(Color)`, `Visible(bool)` — common visual properties shared by all widget types
- **FR-010**: Text widget MUST own the following tagged properties: `Content(string)`, `FontSize(float)`, `TextColor(Color)`, `TextAlign(TextAlign)`, `FontFamily(string)`, `FontWeight(int)`, `LineHeight(float)`, `MaxLines(int)`, `TextDecoration(TextDecoration)`
- **FR-011**: Text widget MUST support `TextAlign(TextAlign)` — kLeft, kCenter, kRight (horizontal), and `kTop`, kCenter, kBottom (vertical)
- **FR-012**: Button MUST inherit from `Text` — all Text properties (Content, FontSize, TextColor, etc.) are automatically available on Button
- **FR-013**: Button MUST support additional tagged properties: `Label(string)` (wraps Content), `OnClick(function)`, `NormalColor(Color)`, `PressedColor(Color)`; Button also inherits `Enabled(bool)` from Widget base
- **FR-014**: Image widget MUST support `FitMode(FitMode)` — kFill, kContain, kCover, kStretch
- **FR-015**: Image widget MUST support `CornerRadius(float)` for rounded corners
- **FR-016**: All properties MUST be configurable via tagged parameters in the constructor AND via Style
- **FR-017**: Properties MUST NOT break existing widget API — all existing constructors remain valid

### Key Entities

- **Style**: A reusable bundle of visual and typographic properties. Supports chainable setters. Per-property `is_set` flag enables priority layering. Four priority levels (low→high): Global default → Class default → Instance Style → Explicit tags. `Style::SetDefault()` sets global theme (main-thread-only, affects future widgets only). `ApplyStyle()` merges instance Style into widget.
- **Text**: Inherits Widget base properties (Width, Height, Background, etc.) plus owns typographic properties (FontSize, TextColor, TextAlign, FontFamily, FontWeight, LineHeight, MaxLines, TextDecoration).
- **Button**: Inherits from Text — gets all Text + Widget properties automatically. Adds interactive properties: OnClick, NormalColor, PressedColor. Inherits Enabled from Widget base (Disabled behavior unified across all widgets). Button's Draw uses state color for background and Text properties for label rendering.
- **Image Widget**: Displays images with FitMode control. Supports CornerRadius for rounded corners.
- **Widget Base Properties**: Width, Height, Background, Enabled, Visible, Opacity, CornerRadius, BorderWidth, BorderColor — ALL widget types share these common visual/behavioral properties. Width/Height are CSS-semantic preferred sizes used as Yoga constraints.

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can set `Width(200)` and `Height(48)` on a Container and verify the layout respects the preferred size — testable via Measure output
- **SC-002**: A developer can set `Enabled(false)` on a Button and verify that a mouse event is NOT delivered — testable via callback flag
- **SC-003**: A developer can set Opacity(0.5) on any widget and verify the output pixels have 50% reduced alpha — testable via pixel readback
- **SC-004**: A developer can set Background + CornerRadius on any Container and verify rounded rect rendering — testable via pixel readback
- **SC-005**: Text with FontWeight(700) renders visibly thicker glyphs than FontWeight(400) — testable via pixel comparison
- **SC-006**: Image with FitMode(kCover) fills the entire widget bounds — testable via edge pixel verification
- **SC-007**: All new properties are settable via tagged parameters and do not break existing construction patterns — verified by existing test suite
- **SC-008**: A Text widget with MaxLines(1) and overflow truncates content with an ellipsis — testable via pixel readback

## Assumptions

- Style is a plain data class with per-property `is_set` flags and value semantics (copyable)
- Style::SetDefault() is main-thread-only and only affects widgets created AFTER the call
- ApplyStyle merges only properties where `is_set == true` — unset properties are ignored (fall through)
- Priority: explicit tags > instance Style > class default > global default
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
- FitMode is implemented via `Canvas::DrawImage` with source/dest rect transformations
- Visible(false) skips Draw but does NOT remove the widget from layout
- Label("OK") on Button is synonymous with Content("OK") for convenience

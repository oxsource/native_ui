# Feature Specification: Widget Property Enhancement

**Feature Branch**: `009-widget-property-enhancement`

**Created**: 2026-07-30

**Status**: Draft

**Input**: User description: "对基础控件的属性进行丰富完善"

## User Scenarios & Testing

### User Story 1 - Developer Configures Visual Properties on All Widgets (Priority: P1)

A developer sets common visual properties — opacity, visibility, background color, corner radius, and border — on Text, Button, Image, and Container widgets using tagged parameters. These properties affect rendering without requiring custom Draw overrides.

**Why this priority**: Visual properties are the most requested customization for any UI framework. Enabling them on the base Widget level eliminates repetitive code and enables declarative styling.

**Independent Test**: A developer creates a Text widget with Opacity(0.5) and renders it, then verifies the output pixels have reduced alpha compared to an identical widget without opacity.

**Acceptance Scenarios**:

1. **Given** a Text widget with `Opacity(0.5)`, **When** drawn on a Canvas, **Then** the rendered pixels have ~50% alpha compared to the same widget at full opacity
2. **Given** a Container with `Background(kBlue)` and `CornerRadius(8)`, **When** drawn, **Then** a rounded blue rect fills the container bounds
3. **Given** a Button with `BorderWidth(2)` and `BorderColor(kRed)`, **When** drawn, **Then** a 2px red border surrounds the button
4. **Given** any widget with `Visible(false)`, **When** drawn, **Then** nothing is rendered

---

### User Story 2 - Developer Customizes Text Appearance (Priority: P1)

A developer configures advanced text properties: font family, alignment, line height, text overflow behavior, and text decoration.

**Why this priority**: Text rendering is the most common UI element. Rich text properties enable everything from headings with specific fonts to underlined links and truncated labels.

**Independent Test**: A developer creates two Text widgets with different FontFamily and FontWeight, renders them, and verifies the pixel output differs in glyph shapes and stroke width.

**Acceptance Scenarios**:

1. **Given** a Text widget with `FontFamily("Helvetica")` and `FontWeight(700)`, **When** drawn, **Then** the text renders in bold Helvetica
2. **Given** a Text widget with `TextAlign(kCenter)`, **When** drawn in a wide container, **Then** the text is horizontally centered
3. **Given** a Text widget with `LineHeight(1.5)` and multi-line content, **When** drawn, **Then** line spacing is 1.5x the font size
4. **Given** a Text widget with `MaxLines(1)` and long content, **When** drawn, **Then** the text is truncated with an ellipsis

---

### User Story 3 - Developer Adds Interactive Button States (Priority: P2)

A developer configures Button colors for normal, pressed, hovered, and disabled states. The Button automatically switches colors based on interaction state.

**Why this priority**: Interactive feedback is essential for a polished UI. Buttons without state colors feel flat and provide no visual feedback to user actions.

**Independent Test**: A developer creates a Button with different NormalColor and PressedColor, triggers a press via event dispatch, and verifies the rendered pixels match the pressed color.

**Acceptance Scenarios**:

1. **Given** a Button with `NormalColor(kBlue)` and `PressedColor(kDarkGray)`, **When** a MouseEvent is pushed at the button position, **Then** the button renders in dark gray while pressed
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
- What happens when BorderWidth exceeds the widget dimensions?
- What happens when FontFamily specifies a font that is not installed?
- What happens when MaxLines is 0 or negative?
- What happens when an image FitMode is applied to a zero-sized widget?
- What happens when a Button is both Disabled and pressed?

## Requirements

### Functional Requirements

- **FR-001**: Widget base MUST support `Opacity(float)` — a multiplier (0.0–1.0) applied to all drawn content's alpha channel
- **FR-002**: Widget base MUST support `Visible(bool)` — when false, Draw is a no-op; the widget still occupies layout space
- **FR-003**: Widget base MUST support `Background(Color)` — fills widget bounds with the specified color before child content draws
- **FR-004**: Widget base MUST support `CornerRadius(float)` — rounds corners of the background rect (and border if present)
- **FR-005**: Widget base MUST support `BorderWidth(float)` and `BorderColor(Color)` — draws a stroked rect around widget bounds
- **FR-006**: Text widget MUST support `FontFamily(string)`, `FontWeight(int)`, `FontStyle(FontStyle)`, `TextAlign(TextAlign)`, `LineHeight(float)`, `MaxLines(int)` for advanced text layout
- **FR-007**: Text widget MUST support `TextDecoration(TextDecoration)` — underline, line-through, none
- **FR-008**: Button widget MUST support `NormalColor(Color)`, `PressedColor(Color)`, `HoverColor(Color)`, `Disabled(bool)` for interactive state styling
- **FR-009**: Image widget MUST support `FitMode(FitMode)` — kFill, kContain, kCover
- **FR-010**: Image widget MUST support `CornerRadius(float)` and `BorderWidth/Color` for rounded image display
- **FR-011**: All properties MUST be configurable via tagged parameters in the constructor
- **FR-012**: Properties MUST NOT break existing widget API — all existing constructors remain valid

### Key Entities

- **Common Visual Properties**: Opacity, Visible, Background, CornerRadius, BorderWidth, BorderColor — stored in Widget base class, affecting all widget types
- **Advanced Text Properties**: FontFamily, FontWeight, FontStyle, TextAlignment, LineHeight, MaxLines, TextDecoration — affect Canvas::DrawText behavior
- **Button State Colors**: NormalColor, PressedColor, HoverColor, Disabled — determine Button rendering based on interaction state
- **Image Fit Modes**: FitMode enum (kFill, kContain, kCover) — control how images scale within widget bounds

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can set Opacity(0.5) on any widget and verify the output pixels have 50% reduced alpha — testable via pixel readback
- **SC-002**: A developer can set Background + CornerRadius on any Container and verify rounded rect rendering — testable via pixel readback
- **SC-003**: Text with FontWeight(700) renders visibly thicker glyphs than FontWeight(400) — testable via pixel comparison
- **SC-004**: Button with Disabled(true) does not invoke OnClick when a MouseEvent is pushed — testable via callback flag
- **SC-005**: Image with FitMode(kCover) fills the entire widget bounds — testable via edge pixel verification
- **SC-006**: All new properties are settable via tagged parameters and do not break existing construction patterns — verified by existing test suite
- **SC-007**: A Text widget with MaxLines(1) and overflow truncates content with an ellipsis — testable via pixel readback

## Assumptions

- Opacity is implemented as a canvas alpha multiplier applied via `Canvas::SaveLayer` or paint alpha, not by modifying individual pixel values
- CornerRadius uses Skia's `SkRRect` for drawing rounded rects
- Font properties are passed to Skia's `SkFont` and `SkParagraph` (TextLayout system from post-MVP P9) — if SkParagraph is not yet available, a best-effort subset is implemented with `SkFont`
- Button state colors default to reasonable values (light gray for normal, darker gray for pressed) if not specified
- FitMode is implemented via `Canvas::DrawImage` with source/dest rect transformations
- Visible(false) skips Draw but does NOT remove the widget from layout — the widget still occupies space
- Border is drawn INSIDE the widget bounds (inset), not outside
- The base Widget class gains a `PaintProperties` struct to avoid adding dozens of individual member fields

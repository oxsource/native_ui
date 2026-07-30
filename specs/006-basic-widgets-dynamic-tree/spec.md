# Feature Specification: Basic Widgets & Dynamic Tree

**Feature Branch**: `006-basic-widgets-dynamic-tree`

**Created**: 2026-07-30

**Status**: Draft

**Input**: User description: "Phase 6: Basic Widgets & Dynamic Tree + Data Binding Integration"

## Clarifications

### Session 2026-07-30

- Q: 区别于普通的Image，对于需要绘制hardware buffer的是不是需要单独的控件 → A: 是，新增ExternalImage控件，与基于文件的Image分离。ExternalImage处理平台硬件缓冲区（IOSurface/AHardwareBuffer/DMA-BUF）的逐帧渲染，Image保持为静态文件图片解码控件。

## User Scenarios & Testing

### User Story 1 - Developer Creates Text Widget with Data Binding (Priority: P1)

A developer creates a `Text` widget displaying dynamic content bound to a `State` property. When the underlying data changes, the text updates automatically without manual redraw calls.

**Why this priority**: Text is the most fundamental visual widget — every UI needs labels, headings, and dynamic text content. Data binding integration is the core differentiator of Phase 6.

**Independent Test**: A developer creates a Text widget with content bound to a State property, changes the property value, and verifies the rendered pixel output reflects the new text — all via unit test with surface readback.

**Acceptance Scenarios**:

1. **Given** a Text widget with `Content("Hello")`, **When** drawn on a Canvas, **Then** the text renders at the expected position and size
2. **Given** a Text widget watching a State property, **When** the property value changes, **Then** the widget redraws with the new text content
3. **Given** a Text widget with a font size specified, **When** drawn, **Then** the rendered text matches the specified size
4. **Given** a Text widget with a color specified, **When** drawn, **Then** the text color matches the specified value

---

### User Story 2 - Developer Creates Button with Hit Detection (Priority: P1)

A developer creates a `Button` widget with a label, defines a click callback, and the button detects taps within its bounds and invokes the callback.

**Why this priority**: Buttons are the primary interaction mechanism. Hit detection must work independently since the full event system (P7) depends on P6.

**Independent Test**: A developer creates a Button with a label and click callback, simulates a tap within the button bounds, and verifies the callback is invoked with the correct payload.

**Acceptance Scenarios**:

1. **Given** a Button with `Label("OK")`, **When** drawn on a Canvas, **Then** the label text renders centered within the button area
2. **Given** a Button with a click callback, **When** a point inside the button bounds is tested, **Then** the callback is invoked
3. **Given** a Button with a click callback, **When** a point outside the button bounds is tested, **Then** the callback is not invoked
4. **Given** a Button bound to a State property via its label, **When** the State changes, **Then** the button redraws with the updated label

---

### User Story 3 - Developer Displays Images From Files (Priority: P2)

A developer loads an image from a file path and displays it within an `Image` widget, with the image decoding handled automatically.

**Why this priority**: Image rendering is essential for icons, photos, and visual content. File-based loading is the most common use case.

**Independent Test**: A developer creates an Image widget with a valid file path, draws it on a Canvas, and verifies the pixel output matches the expected image dimensions and content.

**Acceptance Scenarios**:

1. **Given** an Image widget with a valid file path to a PNG, **When** drawn on a Canvas, **Then** the image renders at the specified size
2. **Given** an Image widget with a nonexistent file path, **When** drawn, **Then** nothing is rendered and no crash occurs
3. **Given** an Image widget with an invalid/corrupted file, **When** drawn, **Then** nothing is rendered and no crash occurs

---

### User Story 4 - ExternalImage Renders Hardware Buffers (Priority: P1)

A developer displays a live camera preview or AI inference output by creating an `ExternalImage` widget bound to a hardware buffer. The widget renders the buffer contents each frame without CPU-side pixel copies.

**Why this priority**: Hardware buffer rendering (camera, video decode, ML inference) is a core use case for the framework — these are zero-copy, GPU-side operations fundamentally different from file-based image decoding.

**Independent Test**: A developer creates an ExternalImage widget with a valid HardwareBuffer, draws it on a Canvas, and verifies the pixel output matches the buffer contents — tested via unit test with synthetic buffer.

**Acceptance Scenarios**:

1. **Given** an ExternalImage widget with a valid HardwareBuffer, **When** drawn on a Canvas, **Then** the buffer contents render at the widget's layout bounds
2. **Given** an ExternalImage widget watching a `Property<HardwareBuffer>`, **When** the property is updated with a new buffer, **Then** the widget redraws with the new buffer contents
3. **Given** an ExternalImage widget with an invalid HardwareBuffer, **When** drawn, **Then** nothing is rendered and no crash occurs
4. **Given** an ExternalImage widget with `SetBuffer(HardwareBuffer)`, **When** called multiple times, **Then** each call triggers `RequestRedraw` and the latest buffer is rendered on the next frame

---

### User Story 5 - Developer Stacks Widgets in Z-Order (Priority: P2)

A developer uses `Stack` to layer multiple widgets on top of each other, controlling which widget appears on top.

**Why this priority**: Stack enables overlay UIs — tooltips, badges, floating buttons, and layered compositions that cannot be achieved with flexbox layout alone.

**Independent Test**: A developer creates a Stack with two overlapping children, draws them, and verifies the top child renders over the bottom child in the correct z-order.

**Acceptance Scenarios**:

1. **Given** a Stack with two children (child A at index 0, child B at index 1), **When** drawn, **Then** child B renders on top of child A
2. **Given** a Stack with three children, **When** drawn, **Then** they render in order: index 0 = bottom, index N = top
3. **Given** a Stack with overlapping children, **When** drawn, **Then** the top child fully occludes the bottom child in overlap regions

---

### Edge Cases

- What happens when Text content is an empty string?
- What happens when Text font size is set to zero or negative?
- What happens when Button label is empty?
- What happens when Image file path is empty?
- What happens when Image widget dimensions are zero?
- What happens when ExternalImage receives a null HardwareBuffer?
- What happens when ExternalImage buffer is replaced mid-frame?
- What happens when ExternalImage is destroyed while the buffer producer is still active?
- What happens when Stack has zero children?
- What happens when AddChild is called during a draw cycle?
- What happens when a watched State property is destroyed while a widget still references it?

## Requirements

### Functional Requirements

- **FR-001**: Text widget MUST support tagged-parameter construction with `Content(string)`, font size, and color
- **FR-002**: Text widget MUST support `Id(string)` for widget tree lookup via `FindById`
- **FR-003**: Text widget MUST support `Watch(Property<T>&)` to bind to State properties and auto-redraw on change
- **FR-004**: Text widget MUST implement `Draw(Canvas&)` to render text using the framework's text rendering API
- **FR-005**: Button widget MUST support tagged-parameter construction with `Label(string)` and an `OnClick` callback
- **FR-006**: Button widget MUST support `Id(string)` for widget tree lookup
- **FR-007**: Button widget MUST provide hit testing — a method to determine if a given Point falls within its bounds
- **FR-008**: Button widget MUST invoke the `OnClick` callback when a hit is detected within its bounds
- **FR-009**: Button widget MUST support `Watch(Property<T>&)` on its label for dynamic label content
- **FR-010**: Image widget MUST support tagged-parameter construction with `ImagePath(string)` for file-based images
- **FR-011**: Image widget MUST decode image files (PNG, JPEG) and render them via `Draw(Canvas&)`
- **FR-012**: Image widget MUST handle missing or invalid file paths gracefully — no crash, no render
- **FR-013**: Stack widget MUST support tagged-parameter construction with `Children{...}`
- **FR-014**: Stack widget MUST render children in z-order: index 0 = bottom, highest index = top
- **FR-015**: Stack widget MUST support `AddChild(unique_ptr<Widget>)` and `RemoveChild(Widget*)` for dynamic tree manipulation
- **FR-016**: All container widgets (Container, Stack) MUST trigger `RequestLayout()` after `AddChild` or `RemoveChild`
- **FR-017**: All widgets MUST support `Watch(Property<T>&)` to integrate with the State data binding system — property changes trigger `RequestRedraw`
- **FR-018**: Dynamic tree operations (AddChild, RemoveChild) MUST work correctly with data-bound children — adding a widget that watches State should trigger layout and render
- **FR-019**: ExternalImage widget MUST support tagged-parameter construction with `HardwareBuffer(HardwareBuffer)` for hardware buffer rendering
- **FR-020**: ExternalImage widget MUST support `SetBuffer(HardwareBuffer)` to update the buffer post-construction, triggering `RequestRedraw`
- **FR-021**: ExternalImage widget MUST support `Watch(Property<HardwareBuffer>&)` to bind to a State property and auto-redraw on buffer change
- **FR-022**: ExternalImage widget MUST render the hardware buffer via `Canvas::DrawImage(Image::FromBuffer(...))` — zero CPU-side pixel copy
- **FR-023**: ExternalImage widget MUST handle invalid/null HardwareBuffer gracefully — no crash, no render

### Key Entities

- **Text**: A leaf widget that renders a string of text with configurable font size and color. Supports data binding via `Watch`.
- **Button**: A leaf widget that renders a clickable label area. Provides hit testing and an `OnClick` callback. Supports data binding on its label.
- **Image**: A leaf widget that decodes and renders image files (PNG, JPEG) within its bounds. Handles missing/invalid files gracefully.
- **ExternalImage**: A leaf widget that renders platform hardware buffers (IOSurface, AHardwareBuffer, DMA-BUF) via GPU zero-copy. Supports per-frame buffer updates via `SetBuffer` or `Watch(Property<HardwareBuffer>&)`.
- **Stack**: A container widget that layers children in z-order. Does not use Yoga/flexbox — children are positioned by index order.
- **Widget Watch**: The mechanism by which a widget subscribes to State property changes. When the property changes, the widget is marked dirty and redrawn on the next frame.

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can create a Text widget, bind it to a State property, change the property, and see the text update — all within 5 minutes of API reference
- **SC-002**: Button hit detection correctly identifies hits inside vs outside bounds with 100% accuracy in unit tests
- **SC-003**: Image widget loads and renders PNG and JPEG files without crash — missing files produce no visible output and no error cascade
- **SC-004**: Stack renders 3+ layered children in correct z-order verified by pixel readback tests
- **SC-005**: Dynamic AddChild on any container triggers re-layout and re-render — verified within a single frame cycle
- **SC-006**: A widget watching a State property redraws automatically when the property changes — verified by pixel change detection
- **SC-007**: All edge cases (empty text, zero-size, missing files, empty stack, null hardware buffer) handled without crash
- **SC-008**: ExternalImage renders a hardware buffer with zero CPU-side pixel copies — verified by buffer ownership tracking in unit tests

## Assumptions

- Text uses Skia's simple DrawText API — TextLayout (SkParagraph) is explicitly deferred post-MVP
- Default font is the system sans-serif at 16px with black color unless specified
- Button hit testing is self-contained within the Button widget — the full Event System (P7) is not required for basic click detection
- Image decoding is synchronous on the main thread for MVP — async decoding is a future optimization
- Image widget size is determined by the parent layout (flexbox or stack) — intrinsic image size is used only as a natural size hint
- Stack has no built-in scrolling or panning — children are positioned at (0,0) offset relative to the Stack's bounds
- Dynamic tree operations (AddChild, RemoveChild) are not thread-safe — they are called from the main thread only
- The State system from Phase 3 provides the Watch mechanism — Property<T> signals trigger RequestRedraw on the watching widget
- All widgets in Phase 6 are clip children to their bounds by default
- ExternalImage is a separate widget from Image — file-based static images and hardware buffer dynamic images have fundamentally different lifecycle and construction patterns
- ExternalImage renders hardware buffers via `Image::FromBuffer()` each frame — the buffer is imported as a GPU texture without host-side copy
- ExternalImage does not own the underlying platform buffer; the producer retains ownership and lifetime management

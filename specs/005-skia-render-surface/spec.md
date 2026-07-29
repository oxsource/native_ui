# Feature Specification: Skia Render Wrapper & Surface

**Feature Branch**: `005-skia-render-surface`

**Created**: 2026-07-29

**Status**: Draft

**Input**: User description: "Skia Render Wrapper & Surface — Phase 5"

## User Scenarios & Testing

### User Story 1 - Developer Renders Shapes with Canvas (Priority: P1)

A developer uses `Canvas` to draw rectangles, text, and paths on a `Surface`. The Canvas automatically saves and restores state on scope exit, preventing state leaks.

**Why this priority**: The Canvas is the core rendering interface that every `Widget::Draw()` call uses. Without it, no widget can render its content.

**Independent Test**: A developer creates a Surface, attaches a Canvas, draws a rect with a Paint, verifies the pixel output, and confirms the Canvas state is restored after scope exit — all via unit tests with pixel readback.

**Acceptance Scenarios**:

1. **Given** a Surface, **When** a Canvas is attached and DrawRect is called, **Then** pixels at the rect position match the expected color
2. **Given** a Canvas with Save/Restore pairs, **When** nested state changes are made, **Then** the final state matches the entry state
3. **Given** a Canvas scope exit, **When** the Canvas is destroyed, **Then** the SkCanvas state is automatically restored

---

### User Story 2 - Developer Draws Images from Various Sources (Priority: P1)

A developer loads images from encoded data (PNG, JPEG), from platform hardware buffers (camera preview), and from SVG, and draws them onto a Canvas.

**Why this priority**: Image rendering is required for all non-trivial UIs — icons, photos, camera feeds, and vector graphics.

**Independent Test**: Each Image source type (encoded file, hardware buffer, SVG) has a corresponding unit test that verifies correct decoding, size readback, and DrawImage pixel output.

**Acceptance Scenarios**:

1. **Given** encoded PNG data, **When** `Image::FromEncoded` is called, **Then** an Image is returned with the correct dimensions
2. **Given** a HardwareBuffer from a camera, **When** `Image::FromBuffer` is called and drawn, **Then** the image renders without error
3. **Given** SVG XML content, **When** `Image::FromFile` detects an SVG file, **Then** it is parsed and rasterized at the specified size

---

### User Story 3 - Surface Manages Display and External Buffers (Priority: P2)

A developer creates Surfaces for both on-screen rendering and off-screen / platform buffer rendering.

**Why this priority**: Surface is the bridge between Skia's rendering and the platform's display or buffer pipeline. Off-screen surfaces enable pre-rendering and caching.

**Independent Test**: Surface creation from a synthetic buffer descriptor produces a valid output that can be rendered to and read back.

**Acceptance Scenarios**:

1. **Given** width and height, **When** `Surface::Create` is called, **Then** a valid Surface is returned with matching dimensions
2. **Given** a HardwareBuffer descriptor, **When** `Surface::CreateFromBuffer` is called, **Then** a Surface wrapping the buffer is returned

---

### Edge Cases

- What happens when DrawRect coordinates are negative or zero-sized?
- What happens when Image::FromFile is called with a nonexistent path?
- What happens when HardwareBuffer is null or invalid?
- What happens when Canvas APIs are called after the Surface is destroyed?

## Requirements

### Functional Requirements

- **FR-001**: Canvas must attach to a Surface via `explicit Canvas(Surface&)` and auto-restore on destruction
- **FR-002**: Canvas must support DrawRect, DrawText, DrawPath, DrawImage
- **FR-003**: Canvas must support Save, Restore, ClipRect, Translate state management
- **FR-004**: Paint must support chainable SetColor, SetAntiAlias, SetStrokeWidth, SetStyle, SetAlpha
- **FR-005**: Path must support MoveTo, LineTo, CubicTo, Close
- **FR-006**: Image must support FromEncoded (PNG/JPEG/WebP/SVG auto-detect), FromFile, FromBuffer (HardwareBuffer)
- **FR-007**: Surface must support Create(width, height), CreateFromBuffer(HardwareBuffer), Flush
- **FR-008**: HardwareBuffer must wrap AHardwareBuffer (Android), IOSurface (macOS), DMA-BUF fd (Linux)
- **FR-009**: Public headers must re-export render types (Surface, Image, Canvas, Paint, Path) and HardwareBuffer

### Key Entities

- **Canvas**: RAII drawing context attached to a Surface — all drawing operations go through Canvas
- **Surface**: Backing store (pixel buffer) — owns the SkSurface, Flush commits pixels
- **Image**: A drawable image source — decoded from file, encoded data, or hardware buffer
- **Paint**: Drawing style configuration — color, stroke, anti-alias
- **Path**: Vector path for custom shapes
- **HardwareBuffer**: Type-erased cross-platform hardware buffer wrapper

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can draw a rect, verify pixel output, and confirm Canvas auto-restore in under 10 minutes
- **SC-002**: Image decoding works for at least 2 formats (PNG, JPEG) with correct dimensions; nonexistent files handled without crash
- **SC-003**: Canvas state (Save/Restore) nesting at least 8 levels deep works correctly
- **SC-004**: Surface creation from a HardwareBuffer does not crash when buffer is valid

## Assumptions

- Skia is already integrated as an external dependency from Phase 1
- Canvas is a lightweight RAII wrapper — no virtual methods, no heap allocation per frame
- Image decoding is deferred (lazy) — actual decode happens on first DrawImage call
- TextLayout is explicitly out of scope — DrawText uses Skia's simple text API
- HardwareBuffer is header-only with platform `#ifdef` dispatch
- The PlatformSurface widget (which wraps Surface for widget tree) is in Phase 6 scope

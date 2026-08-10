# Feature Specification: Android AHardwareBuffer ExternalImage

**Feature Branch**: `011-ahwb-external-image`

**Created**: 2026-08-03

**Status**: Draft

**Input**: User description: "以安卓平台的AHardwareBuffer为依据，参考falcon/core/utils/ahwb.cc实现ExternalImage，并新增提案完善ExternalImage在安卓侧的实现"

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Display an external image buffer on Android (Priority: P1)

A developer integrating the framework on Android receives an image produced by the platform — for example a decoded media frame or a camera preview frame — as an Android hardware-backed image buffer (`AHardwareBuffer`). The developer hands this buffer to an `ExternalImage` widget in their UI tree. The widget displays the image content on screen, replacing the current behavior where such buffers silently render nothing.

**Why this priority**: This is the core value of the feature. Today the widget exists but every buffer renders nothing (the buffer-to-image conversion is unimplemented), so no producer can display an external frame. Without this story nothing else delivers value.

**Independent Test**: On an Android device/emulator, an app creates a widget bound to a known hardware-backed buffer containing recognizable pixel content (e.g., a solid color or test pattern). The widget shows the correct content on screen, matching the source pixels.

**Acceptance Scenarios**:

1. **Given** a valid Android hardware-backed image buffer containing a known RGBA test pattern, **When** the developer binds it to the widget and renders one frame, **Then** the on-screen output matches the source content exactly (color and geometry).
2. **Given** a buffer whose visible width is narrower than its allocated row width (row padding), **When** the widget renders it, **Then** the output shows only the visible content with no horizontal smearing or artifact bands.
3. **Given** an invalid or empty buffer, **When** the developer binds it and renders, **Then** the widget draws nothing and the application neither crashes nor corrupts the rest of the UI.

---

### User Story 2 - Live-updating external frames (Priority: P2)

The developer displays a continuously changing source, such as a live camera preview or video playback. New frames arrive frequently as hardware-backed buffers. The developer binds a watchable buffer property to the widget; as the property value changes, the widget updates its display accordingly.

**Why this priority**: Live preview/playback is the primary real-world use of external image buffers and exercises the update path. It builds directly on Story 1 (which covers a single static frame).

**Independent Test**: An app updates the bound buffer value at a sustained rate (e.g., 30 times per second) for a fixed duration while rendering; the widget keeps showing current content and the application stays responsive.

**Acceptance Scenarios**:

1. **Given** a widget bound to a watchable buffer property, **When** the property is updated with a new buffer, **Then** the widget reflects the new content on its next rendered frame without user interaction.
2. **Given** buffers arriving at 30 updates per second for 60 seconds, **When** the app renders continuously, **Then** the displayed content tracks the updates and the app remains interactive (no observable freeze).
3. **Given** a producer that stops sending new buffers, **When** the app keeps rendering, **Then** the last displayed frame persists without glitches or crashes.

---

### User Story 3 - Producer utilities and diagnostics for external buffers (Priority: P3)

A developer who produces external image buffers needs supporting helpers and diagnostics: creating a hardware-backed buffer from in-memory pixel data, and exporting the currently displayed content to a viewable image file for verification/debugging.

**Why this priority**: These helpers mirror the reference utility layer and complete the Android-side story, but they are not required for the core display and live-update paths; hence lowest priority.

**Independent Test**: A test app creates a hardware-backed buffer from known pixel data, displays it, and writes the displayed content to a PNG file; the file, when opened, shows the same content as the source pixels.

**Acceptance Scenarios**:

1. **Given** in-memory RGBA pixel data and dimensions, **When** a producer creates a hardware-backed buffer from it, **Then** the resulting buffer is valid and displays the correct content in the widget.
2. **Given** a widget currently displaying a frame, **When** the developer requests a diagnostic export of that frame, **Then** a viewable image file is produced whose pixels match the displayed content.
3. **Given** a buffer in an unsupported pixel format, **When** the developer attempts to display or export it, **Then** the framework reports a clear, observable error state and does not render corrupt output.

---

### Edge Cases

- What happens when a producer releases/abandons a buffer while the widget still references it? (Must not use-after-free; widget keeps last safe content.)
- What happens when the buffer row stride exceeds the visible width? (Padding must be ignored, not rendered.)
- What happens when the buffer dimensions exceed or fall below the widget's layout bounds? (Existing scaling rules apply; no crash.)
- What happens when a new buffer arrives during an in-progress render? (Frame consistency: no torn or mixed-frame output.)
- What happens when an unsupported pixel format is supplied? (Defined error, no corrupt output, no crash.)
- What happens when the buffer is valid but has zero area? (Nothing rendered, no crash.)
- What happens under sustained updates — does memory stay bounded? (No unbounded growth/leaks.)

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST display the content of an externally provided Android hardware-backed image buffer as an on-screen image when the buffer uses a supported pixel format.
- **FR-002**: The system MUST correctly render buffers whose visible width is smaller than their allocated row width (row padding), showing only the visible content without artifacts.
- **FR-003**: The system MUST update the displayed image when the widget's bound buffer changes, on the next rendered frame, without user interaction.
- **FR-004**: The system MUST keep memory usage bounded across repeated buffer updates; 10,000 successive updates MUST NOT grow application memory unboundedly (no leak).
- **FR-005**: The system MUST NOT crash or corrupt output when given an invalid, empty, zero-area, or already-released buffer; the widget renders nothing instead.
- **FR-006**: The system MUST handle unsupported buffer pixel formats with a defined, observable error state and MUST NOT render corrupt output.
- **FR-007**: The system MUST complete rendering of a typical-size frame (e.g., 1080p) within the platform display frame budget so that the application UI remains responsive during live updates.
- **FR-008**: The system MUST render content consistent within a single frame when a new buffer arrives during an in-progress draw (no mixing of two buffers).
- **FR-009**: The system MUST provide a way to create a hardware-backed image buffer from in-memory pixel data for use by producers.
- **FR-010**: The system MUST provide a diagnostic capability that exports the currently displayed frame to a viewable image file for verification.
- **FR-011**: The lifetime of a producer-supplied buffer MUST remain with the producer; the system MUST copy or otherwise safely capture the content it renders so later producer actions do not corrupt the display.
- **FR-012**: The system MUST preserve existing behavior on non-Android platforms (no regression to currently supported rendering paths).

### Key Entities *(include if feature involves data)*

- **Hardware-backed image buffer**: The cross-platform representation of a platform image buffer (on Android, the platform's hardware buffer type). Attributes: validity, pixel format, dimensions, row stride, and underlying platform buffer handle. Produced by the platform/app and consumed by the widget.
- **ExternalImage widget**: The UI element that binds a hardware-backed image buffer to display. Attributes: currently bound buffer, optional live-binding to a watchable buffer property, and resulting drawable content.
- **Renderable image**: The drawable resource produced from a buffer that the renderer consumes. Created from valid buffers and released/replaced on buffer change.
- **Buffer property binding**: The declarative link a widget can establish to a buffer value, so content updates flow automatically.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: An app can display an externally provided hardware-backed buffer on Android within one frame of supplying it (100% of static-frame test cases show correct content).
- **SC-002**: Live buffer updates are reflected on screen within one rendered frame of delivery for at least 95% of updates during a sustained 30 FPS, 60-second run.
- **SC-003**: The app remains responsive (no observable freezes) while rendering 30 buffer updates/second for 60 seconds.
- **SC-004**: Application memory stays bounded over 10,000 successive buffer updates (no measurable unbounded growth attributable to the feature).
- **SC-005**: 100% of invalid/empty/released-buffer test cases render nothing without crashing or corrupting the UI.
- **SC-006**: Diagnostic exports reproduce the displayed content pixel-for-pixel for supported formats in 100% of test cases.
- **SC-007**: No regressions on non-Android platforms: existing image/UI tests continue to pass unchanged.

## Assumptions

- Android is the only platform in scope for this feature; macOS and Linux keep their existing (stubbed) behavior for external buffers, with no regressions.
- Version 1 uses a CPU-based transfer path consistent with the reference implementation (lock → transfer → unlock); zero-copy GPU texturing is explicitly out of scope and may be proposed later.
- The supported pixel format for version 1 is 8-bit RGBA with alpha (4 bytes per pixel), which matches the render pipeline's native format. Other formats such as YUV 4:2:2 are handled via best-effort conversion or explicit rejection (FR-006).
- Buffer producers retain ownership of the buffers they supply; the widget captures the content it renders (FR-011).
- Existing widget scaling rules apply when buffer and widget dimensions differ.
- Verification happens on an Android device/emulator; where the build environment permits, synthetic pixel data is used to validate conversion logic on host platforms to keep CI coverage.

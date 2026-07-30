# Research: Basic Widgets & Dynamic Tree

**Date**: 2026-07-30

## Decisions

### Button Hit Testing: Self-Contained (No EventHub)

- **Decision**: Button widget implements hit testing internally via a `HitTest(Point) -> bool` method. The Button stores its layout bounds (assigned by parent Container after Arrange) and checks `bounds_.Contains(point)`.
- **Rationale**: The Event System (P7) depends on P6, so Button must work without it. Self-contained hit testing avoids circular dependency. The EventHub (P7) will later supersede this for general event dispatch.
- **Alternatives considered**: Expose generic HitTest on Widget base (rejected: only Button needs it, YAGNI); Wait for P7 (rejected: P7 depends on P6)

### Text Rendering: Simple DrawText API

- **Decision**: Text widget uses `Canvas::DrawText(string, Point, Paint)` which wraps Skia's simple text drawing (SkFont + SkTextBlob). No SkParagraph, no RichText, no text selection.
- **Rationale**: TextLayout (SkParagraph, ICU, harfbuzz) is explicitly deferred post-MVP. Simple text is sufficient for labels, buttons, and dynamic content in MVP.
- **Alternatives considered**: SkParagraph integration (deferred post-MVP); Custom text layout (rejected: NIH, high risk)

### Image Loading: Synchronous Decode

- **Decision**: Image widget calls `Image::FromFile(path)` which synchronously decodes PNG/JPEG via Skia. Decoding happens on the main thread at widget creation time.
- **Rationale**: The existing Image class already provides synchronous `FromFile`. Async decoding is a future optimization. For MVP, images are expected to be small (icons, thumbnails).
- **Alternatives considered**: Lazy decode on first Draw (rejected: Image already decodes eagerly); Async with callback (deferred post-MVP)

### Stack Layout: No Yoga, Index-Order Z-Stacking

- **Decision**: Stack widget maintains a `vector<unique_ptr<Widget>> children_` and draws them in index order (0 = bottom, N = top). No Yoga nodes involved. Stack size equals the largest child's size.
- **Rationale**: Stack is intentionally non-flexbox — it provides z-order layering for overlays, badges, tooltips. Using Yoga for z-order would be unnatural.
- **Alternatives considered**: Reuse Container with Yoga (rejected: Yoga doesn't do z-order stacking); Positioned widget approach (rejected: over-engineered for MVP)

### Hardware Buffer Widget: Separate from File-Based Image

- **Decision**: `ExternalImage` is a separate widget from `ImageWidget`. It accepts `HardwareBuffer` (or `Property<HardwareBuffer>`) and renders via `Image::FromBuffer()` each frame. File-based `ImageWidget` stays focused on static PNG/JPEG decode.
- **Rationale**: Hardware buffer rendering (camera preview, video decode, ML inference output) has fundamentally different lifecycle — per-frame buffer updates vs. static decode-once. Different construction parameters (buffer descriptor vs. file path), different invalidation patterns (every frame vs. never). Merging them would create a complex class with conditional branches on source type.
- **Alternatives considered**: Unified widget with both `ImagePath` and `HardwareBuffer` tags (rejected: different lifecycle models); Defer hardware buffer widget to post-MVP (rejected: hardware buffer rendering is a core differentiator already built in P5)

### Data Binding: Watch() on Widget, Signal() on Property

- **Decision**: Widget::Watch(Property<T>&) registers the widget with the Property's owning State. On Property::operator=, Signal() enqueues the Property in the State's dirty queue. Widget::RequestRedraw is called during State::Flush().
- **Rationale**: This pattern already exists in P3 (State + Property + Widget::Watch). Phase 6 adds the Widget implementations that actually use it — Text watches a string property, Button watches its label property.
- **Alternatives considered**: Direct callback approach (rejected: duplicates existing State system); Polling (rejected: wasteful)

### Canvas Sharing: Single Canvas Per Frame

- **Decision**: All widgets in the tree receive the same `Canvas&` reference during Draw(). Widgets use `canvas.Save()`/`canvas.Restore()` and `canvas.Translate()` to render in local coordinates.
- **Rationale**: Matches the existing Container::Draw pattern established in P3/P5. One Canvas per frame avoids creating multiple drawing contexts.

# Research: Skia Render Wrapper & Surface

**Date**: 2026-07-29

## Decisions

### Canvas: RAII with Save/Restore on Scope

- **Decision**: `Canvas` saves the `SkCanvas` state on construction and restores it on destruction. Nested save/restore pairs are handled via explicit `Save()`/`Restore()`.
- **Rationale**: SkCanvas requires explicit save/restore management — forgetting either causes visual bugs or crashes. RAII eliminates this entire class of errors.
- **Alternatives considered**: No RAII wrapper (rejected: error-prone); Canvas as abstract interface (rejected: unnecessary virtual dispatch for hot path)

### Paint: Header-Only Chainable Builder

- **Decision**: `Paint` is header-only, no `.cc` file. All methods return `*this` for chaining. Wraps SkPaint fields directly.
- **Rationale**: Paint is a value type with no logic beyond setter chaining. Header-only avoids linker overhead.

### Image: Deferred/Lazy Decode

- **Decision**: `Image::FromEncoded` and `Image::FromFile` store the encoded data but do NOT decode until `DrawImage` is called on Canvas. `Image::FromBuffer` wraps a HardwareBuffer without copying pixel data.
- **Rationale**: Avoids decoding images that are never drawn (e.g., in hidden widgets). Defers expensive decode to the render thread.
- **Alternatives considered**: Eager decode on load (rejected: wasted decode for invisible widgets); Decode on constructor (rejected: blocks main thread)

### HardwareBuffer: Type-Erased Variant

- **Decision**: `HardwareBuffer` is a header-only variant that holds one of: `AHardwareBuffer*` (Android), `IOSurfaceRef` (macOS), or `int dma_buf_fd` (Linux). Platform dispatch via `#ifdef`.
- **Rationale**: The surface module must support three platforms with different buffer types. A type-erased variant avoids virtual dispatch and keeps the interface clean.

### SurfaceFactory: Platform Dispatch

- **Decision**: `SurfaceFactory` uses `#ifdef` to select the correct platform backend for creating an `SkSurface` from a `HardwareBuffer`. macOS uses `SkSurface::MakeFromIOSurface`, Linux uses `SkSurface::MakeFromAHardwareBuffer` or custom DMA-BUF path.
- **Rationale**: Encapsulates platform differences behind a single factory interface. The public API (`Surface::CreateFromBuffer`) is platform-agnostic.

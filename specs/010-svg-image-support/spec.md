# Feature Specification: SVG Image Support

**Feature Branch**: `010-svg-image-support`

**Created**: 2026-07-30

**Status**: Draft

**Input**: User description: "验证SVG图像加载支持"

## User Scenarios & Testing

### User Story 1 - Developer Loads and Renders SVG Images (Priority: P1)

A developer creates an `ImageWidget` with an SVG file path, the SVG is parsed and rendered to a Canvas at the widget's bounds. The SVG scales correctly with the widget size.

**Why this priority**: SVG is the most requested vector image format for icons, illustrations, and logos. Without SVG support, developers must manually convert SVGs to PNGs.

**Independent Test**: A developer creates an ImageWidget with a valid SVG file (`icon.svg`), sets Width(100) and Height(100), renders it, and verifies the output pixels contain the expected vector shapes.

**Acceptance Scenarios**:

1. **Given** an ImageWidget with a valid SVG file path, **When** rendered, **Then** the SVG is parsed and drawn at the widget's bounds
2. **Given** an ImageWidget with `ScaleType(kCenterInside)`, **When** rendered at 200×200 with an SVG that has different aspect ratio, **Then** the SVG scales uniformly within bounds
3. **Given** an ImageWidget with an invalid SVG file, **When** rendered, **Then** nothing is drawn and no crash occurs
4. **Given** an SVG with embedded raster images, **When** rendered, **Then** the embedded images are decoded and drawn as part of the SVG

---

### User Story 2 - Developer Adds SVG Loading Example (Priority: P2)

A developer runs a dedicated SVG example that loads and renders multiple SVG files with different scale modes, serving as both documentation and functional validation.

**Why this priority**: A working example demonstrates the SVG pipeline end-to-end and provides a visual reference for users.

**Independent Test**: A developer runs the SVG example binary and verifies it produces PNG output files with rendered SVG content — each testing a different ScaleMode.

**Acceptance Scenarios**:

1. **Given** the SVG example source, **When** built and run, **Then** it produces valid PNG output files with rendered SVG graphics
2. **Given** the SVG example, **When** inspected, **Then** it uses `ImageWidget` with `ImagePath` and at least two different `ScaleType` modes

---

### Edge Cases

- What happens when an SVG file contains unsupported features (e.g., animations, scripts)?
- What happens when an SVG references external files (fonts, images) that don't exist?
- What happens when SVG viewBox is missing or invalid?
- What happens when the SVG file is malformed XML?
- What happens when the SVG dimensions are extremely large (10k×10k+)?
- What happens when a non-SVG file is loaded via ImagePath with .svg extension?

## Requirements

### Functional Requirements

- **FR-001**: `Image::FromFile()` MUST support SVG files — detect `.svg` extension and route to SVG parser instead of raster decoder
- **FR-002**: SVG MUST be parsed and rendered at the widget's target bounds — scaling preserves aspect ratio by default
- **FR-003**: SVG rendering MUST integrate with existing `ImageWidget` — `ScaleType`, `ScaleGravity`, `CornerRadius`, and `Background` all apply to SVG as they do to raster images
- **FR-004**: A dedicated SVG example (`examples/svg_demo.cc`) MUST demonstrate loading and rendering SVG files with different ScaleType modes
- **FR-005**: An invalid or malformed SVG MUST NOT crash the framework — `Image::FromFile` returns nullptr, ImageWidget shows nothing
- **FR-006**: SVG with missing external references (fonts, images) MUST NOT crash — unsupported features are silently ignored
- **FR-007`: The SVG parser library MUST be small, header-only if possible, and not introduce heavy dependencies

### Key Entities

- **SVG Parser**: A lightweight SVG parsing library (nanosvg or similar header-only library) that converts SVG XML into a set of drawing commands
- **SvgImage**: A new Image subclass or rendering path that bridges SVG parse results into Skia draw calls
- **ImageWidget**: Existing — gains SVG support automatically through `Image::FromFile()` routing
- **SVG Example**: A new `examples/svg_demo.cc` that loads test SVG files and renders them with different ScaleType modes

## Success Criteria

### Measurable Outcomes

- **SC-001**: A valid SVG file renders to a Canvas with correct shapes at the specified size — verified by pixel readback
- **SC-002**: An invalid SVG file returns `Image::FromFile` returning nullptr — no crash
- **SC-003**: The SVG example produces at least 3 output PNGs (kCenterCrop, kCenterInside, kFillXY) with visually different results
- **SC-004**: The SVG parser library adds no more than 500KB to the binary size
- **SC-005**: An SVG file renders in under 100ms for typical icon-sized SVGs (<100KB)

## Assumptions

- SVG parsing uses `nanosvg.h` (header-only, ~25KB, zlib license) — no external build step needed
- nanosvg is added as a third-party header under `third_party/nanosvg/` or directly included
- SVG is rasterized to a Skia bitmap at the widget's target resolution — vector precision is lost at render time but resolution matches widget bounds
- Animations, scripts, and external entity references in SVG are NOT supported — only static SVG elements (rect, circle, path, text, etc.)
- SVG files with missing external references (fonts, images) render without those elements but don't crash
- The existing `Image::FromFile()` is extended to detect `.svg` extension — the returned Image object wraps a rasterized bitmap
- nanosvg's rendering is bridged to Skia via `SkBitmap` + `SkImage::MakeFromBitmap`

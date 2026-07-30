# Feature Specification: SVG Image Support

**Feature Branch**: `010-svg-image-support`

**Created**: 2026-07-30

**Status**: Draft

**Input**: User description: "利用assets/photo目录下的png和svg，结合Glide和ImageWidget实现加载及缩放类型验证"

## Clarifications

### Session 2026-07-30

- Q: 验证使用的资源和范围 → A: 使用 `assets/photo/police.png` 和 `assets/photo/superdog.svg`，通过 Glide 异步加载 + ImageWidget 展示，分别测试 PNG 和 SVG 两种格式的多种 ScaleType 模式
- Q: 展示布局和输出格式 → A: 一个 Container(Column) 内排列多个 ImageWidget + Text 标注对，每个 ImageWidget 使用不同的 ScaleType 和图片源；下方用 Text 简要说明用途。整体渲染后输出单张 PNG 到 /tmp/
- Q: SVG和PNG的分配 → A: SVG 只做2个（原始kCenter + 缩放kCenterInside），PNG 做3个（kCenterCrop/kCenterInside/kFillXY），共5张卡片上下排列，单一控件对比

## User Scenarios & Testing

### User Story 1 - Developer Validates Image Loading in Composite Layout (Priority: P1)

A developer runs a demo that displays a grid of ImageWidget + Text label pairs inside a single Container. Each ImageWidget uses a different ScaleType (kCenterCrop, kCenterInside) and image source (PNG, SVG), with a Text label below explaining the purpose. The entire layout is rendered to a single output PNG.

**Why this priority**: This validates the full widget composition pipeline — Container layout, ImageWidget with async loading, Text labels, and ScaleType rendering — all in one scene.

**Independent Test**: A developer runs the demo binary and verifies it produces a single output PNG showing 5 image+label cards (2 SVG + 3 PNG) in a column with correct layout and visible content.

**Acceptance Scenarios**:

1. **Given** the demo source, **When** built and run, **Then** it produces `/tmp/image_gallery.png` containing 5 cards stacked vertically
2. **Given** the output PNG, **When** inspected, **Then** it shows `superdog.svg` with `ScaleType(kCenter)` (original size, no scaling) labeled "SVG Original"
3. **Given** the output PNG, **When** inspected, **Then** it shows `superdog.svg` with `ScaleType(kCenterInside)` labeled "SVG Scaled"
4. **Given** the output PNG, **When** inspected, **Then** it shows `police.png` with `ScaleType(kCenterCrop)`, `kCenterInside`, `kFillXY` labeled accordingly
5. **Given** a PNG card with `ScaleType(kCenterCrop)`, **When** rendered, **Then** the image fills the card bounds uniformly (no empty borders)
6. **Given** a PNG card with `ScaleType(kFillXY)`, **When** rendered, **Then** the image stretches to fill bounds (aspect ratio not preserved)

---

### User Story 2 - SVG Parsing via nanosvg (Priority: P1)

An SVG file (`superdog.svg`) is parsed via the nanosvg library and rasterized to a Skia bitmap at the widget's target resolution. The resulting Image is usable by ImageWidget with all ScaleType modes.

**Why this priority**: SVG support requires a parser. nanosvg is chosen for its small footprint (header-only, ~25KB) and zero external dependencies.

**Independent Test**: A developer calls `Image::FromFile("assets/photo/superdog.svg")`, receives a valid Image, renders it, and verifies pixel output matches expected shapes.

**Acceptance Scenarios**:

1. **Given** a valid SVG file parsed via nanosvg, **When** rasterized at 200×200, **Then** the resulting Image has correct dimensions and non-empty pixel content
2. **Given** a malformed SVG file, **When** parsed, **Then** nullptr is returned and no crash occurs

---

### Edge Cases

- What happens when PNG file path is invalid or file doesn't exist?
- What happens when SVG file is malformed XML?
- What happens when SVG viewBox is missing or invalid?
- What happens when SVG references external files (fonts, images) that don't exist?
- What happens when an SVG with unsupported features (animations, scripts) is loaded?
- What happens when Glide::Default() is null (not initialized)?
- What happens when the demo is run from a different working directory where asset paths are invalid?
- What happens when an ImageWidget card has no image loaded (loading state)?
- What happens when the demo Container height/width is too small for 4 cards?

## Requirements

### Functional Requirements

- **FR-001**: `Image::FromFile()` MUST support SVG files — detect `.svg` extension and route to nanosvg parser for rasterization
- **FR-002**: nanosvg MUST be integrated as a header-only third-party library at `third_party/nanosvg/nanosvg.h`
- **FR-003**: SVG MUST be rasterized to a Skia `SkBitmap`/`SkImage` at the widget's target resolution — the resulting `Image` object works with all `ImageWidget` ScaleType modes
- **FR-004**: A dedicated demo (`examples/image_demo.cc`) MUST exist that:
  - Initializes Glide with `DefaultGlide`
  - Loads BOTH `police.png` and `superdog.svg` via `Glide::Load()` (async, worker thread)
  - SVG is parsed and rasterized via nanosvg inside Glide's worker thread — `Image::FromFile` detects `.svg` and calls nanosvg
  - Builds a Container(Column) containing 5 cards in a vertical column:
    - Card 1: SVG Original — `ImageWidget(superdog.svg, ScaleType(kCenter))` + `Text("SVG Original")`
    - Card 2: SVG Scaled — `ImageWidget(superdog.svg, ScaleType(kCenterInside))` + `Text("SVG Scaled")`
    - Card 3: PNG CenterCrop — `ImageWidget(police.png, ScaleType(kCenterCrop))` + `Text("PNG CenterCrop")`
    - Card 4: PNG CenterInside — `ImageWidget(police.png, ScaleType(kCenterInside))` + `Text("PNG CenterInside")`
    - Card 5: PNG FillXY — `ImageWidget(police.png, ScaleType(kFillXY))` + `Text("PNG FillXY")`
  - Each card is a Container(Column) with ImageWidget on top and Text label beneath
  - Outputs a single `/tmp/image_gallery.png` showing the full layout
- **FR-005**: An invalid or malformed SVG MUST NOT crash — parsing returns nullptr, ImageWidget shows nothing
- **FR-006**: SVG with unsupported elements (animations, scripts) MUST be silently ignored without crash
- **FR-007**: The nanosvg library MUST be header-only, ≤50KB, with no external dependencies
- **FR-008**: Glide async loading MUST work with ImageWidget — `ImageURI` tag triggers `Glide::Load()` with callback delivering decoded Image
- **FR-009**: Glide::Load() MUST handle both PNG and SVG files transparently — `Image::FromFile()` called on worker thread detects `.svg` and routes to nanosvg, rasterizes, returns uniform `Image` object

### Key Entities

- **nanosvg**: Header-only SVG parser library (`third_party/nanosvg/nanosvg.h`). Parses SVG XML into a set of shape commands, rasterized to a Skia bitmap.
- **Image::FromFile (SVG path)**: Extended to detect `.svg` extension — routes to nanosvg parser, returns an `Image` wrapping a rasterized bitmap.
- **ImageWidget**: Renders both PNG and SVG via the same `Draw(Canvas&)` path — `ScaleType`, `ScaleGravity` apply uniformly.
- **Glide**: Async image loader — loads PNG files via `Image::FromFile` on worker thread, caches result.
- **Demo Example**: `examples/image_demo.cc` — produces 6 output PNGs showing both PNG and SVG with 3 ScaleType modes each.

## Success Criteria

### Measurable Outcomes

- **SC-001**: The demo produces `/tmp/image_gallery.png` — a valid PNG showing all 5 image+label cards in a vertical column
- **SC-002**: The output PNG contains SVG Original (kCenter, no scaling) and SVG Scaled (kCenterInside) — visibly different
- **SC-003**: The output PNG contains 3 PNG ScaleType variants — kCenterCrop (fills bounds), kCenterInside (uniform), kFillXY (stretched)
- **SC-004**: Each card has a Text label below identifying the image type and ScaleType
- **SC-005**: kCenterCrop fills card bounds completely with no empty borders
- **SC-006**: kCenterInside shows uniform scaling with visible letterboxing
- **SC-007**: A malformed SVG file returns nullptr from Image::FromFile — no crash

## Assumptions

- nanosvg header is placed at `third_party/nanosvg/nanosvg.h` and `third_party/nanosvg/nanosvgrast.h` — both header-only
- SVG rasterization uses nanosvg's rasterizer to produce an RGBA pixel buffer, then wrapped as Skia `SkBitmap` + `SkImage`
- ALL image loading goes through `Glide::Load()` (async worker thread) — NOT synchronous `Image::FromFile` on main thread
- `Image::FromFile` is extended to support SVG via nanosvg — called from within Glide's worker thread
- ImageWidget uses `ImageURI` tag (triggers Glide::Load) for both PNG and SVG — never blocks main thread
- SVG loading uses nanosvg rasterizer (header-only) — called from `Image::FromFile` on Glide's worker thread when `.svg` detected
- `police.png` and `superdog.svg` are referenced by absolute path (`ASSETS_DIR/photos/`) — or by path relative to binary location
- Each card is a Container(Column) containing ImageWidget + Text(Content) — arranged inside a master Container(Column)
- Master Container size is fixed via `Width/Height` tags to accommodate 4 cards in a column
- Animations, scripts, embedded fonts, and external references in SVG are NOT supported by nanosvg — silently ignored
- Demo BUILD.bazel depends on `//src/framework/widgets`, `//src/framework/render`, `//src/framework/utils``

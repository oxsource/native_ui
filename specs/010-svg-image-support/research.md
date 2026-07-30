# Research: SVG Image Support

**Date**: 2026-07-30

## Decisions

### SVG Parser: nanosvg (Header-Only)

- **Decision**: Use nanosvg (`nanosvg.h` + `nanosvgrast.h`) for SVG parsing and rasterization. It's a single-header library with no external dependencies.
- **Rationale**: nanosvg is the most widely used header-only SVG parser in the C++ graphics community. It handles the common SVG feature set (paths, rects, circles, text, gradients) and rasterizes to a raw RGBA pixel buffer. Zero build complexity — just `#include` the header.
- **Alternatives considered**: Skia's SkSVGDOM (not available in current Skia build); lunasvg (larger, build step required); Manual XML parsing (reinventing the wheel)

### SVG Integration Point: Image::FromFile

- **Decision**: Extend `Image::FromFile()` to detect `.svg` extension and call nanosvg parser + rasterizer. The result is an `Image` object wrapping a Skia `SkBitmap` of the rasterized SVG.
- **Rationale**: This is the minimal change — no new widget type, no new API. All existing consumers (`ImageWidget`, `Glide`) get SVG support automatically. `Image::FromFile` already exists in `render/image.cc` and is called by Glide's worker thread.
- **Alternatives considered**: New `SvgImage` class (rejected: unnecessary abstraction); New `Image::FromSvgFile()` static method (rejected: single API is cleaner)

### Rasterization at Target Resolution

- **Decision**: SVG is rasterized to a bitmap at the widget's target width/height. The nanosvg rasterizer produces an RGBA pixel buffer, which is wrapped in a Skia `SkBitmap` and then `SkImage`.
- **Rationale**: nanosvg rasterizes to a specified size — the widget's layout bounds are known at load time. Vector precision is lost but matches the display resolution exactly. No resolution-independent vector storage needed.
- **Alternatives considered**: Vector storage with Skia path objects (rejected: nanosvg doesn't output Skia paths, would need extra conversion); Rasterize at fixed 2x resolution (rejected: memory waste)

### Example: image_gallery.cc

- **Decision**: Create `examples/image_gallery.cc` with 5 ImageWidget+Text cards in a Container(Column). Cards are 2 SVG (kCenter, kCenterInside) + 3 PNG (kCenterCrop, kCenterInside, kFillXY). All loading via Glide async.
- **Rationale**: Validates the full pipeline — Glide async loading, SVG rasterization, ScaleType rendering, widget composition. Single output PNG for easy visual diff.
- **Alternatives considered**: Separate unit tests per component (rejected: doesn't validate integration); Hello World integration (rejected: SVG not related to counter demo)

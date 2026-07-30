# nanosvg Integration Contract

**Purpose**: Define how nanosvg is integrated into the framework for SVG parsing and rasterization.

## Integration via http_archive

nanosvg is fetched as an external Bazel dependency via `http_archive` in `native_ui_deps.bzl`:

```python
# In native_ui_deps.bzl
def _nanosvg():
    http_archive(
        name = "nanosvg",
        urls = ["https://github.com/oxsource/nanovg/archive/refs/tags/v1.0.0.tar.gz"],
        sha256 = "91882cb9ea0f6cb75dfbe3a0d272292b640d237e8892d7252b08e38feb930e6f",
        strip_prefix = "nanovg-1.0.0",
        build_file = "//third_party/nanosvg:BUILD.bazel",
    )
```

## BUILD (third_party/nanosvg/BUILD.bazel)

```python
cc_library(
    name = "nanosvg",
    hdrs = ["src/nanosvg.h", "src/nanosvgrast.h"],
    includes = ["src"],
    visibility = ["//src/framework:__subpackages__"],
)
```

The headers are at `src/nanosvg.h` and `src/nanosvgrast.h` within the tarball (after `strip_prefix`).

## Integration in Image::FromFile

```cpp
// native_ui/src/framework/render/image.cc

#include "src/nanosvg.h"
#include "src/nanosvgrast.h"

static std::unique_ptr<Image> LoadSVG(const char* path, int width, int height) {
  NSVGimage* svg = nsvgParseFromFile(path, "px", 96.0f);
  if (!svg) return nullptr;

  // Use provided size or SVG's natural size
  int w = width > 0 ? width : static_cast<int>(svg->width);
  int h = height > 0 ? height : static_cast<int>(svg->height);
  if (w <= 0 || h <= 0) { nsvgDelete(svg); return nullptr; }

  // Rasterize to RGBA buffer
  unsigned char* rgba = new unsigned char[w * h * 4];
  nsvgRasterize(svg, 0, 0, static_cast<float>(w), static_cast<float>(h), rgba, w * 4);
  nsvgDelete(svg);

  // Wrap in SkBitmap
  SkBitmap bm;
  bm.installPixels(SkImageInfo::Make(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                   rgba, w * 4);
  // SkBitmap takes ownership of rgba via pixel ref
  auto sk_image = SkImage::MakeFromBitmap(bm);
  if (!sk_image) { delete[] rgba; return nullptr; }

  // Wrap in native::ui::Image (via internal API — need to construct from SkImage)
  return Image::FromSkImage(sk_image);
}
```

Note: `Image::FromSkImage` is a new internal factory method that creates an `Image` wrapping an existing `sk_sp<SkImage>`.

## Image::FromFile Extension

```cpp
// In render/image.cc, existing FromFile:
std::unique_ptr<Image> Image::FromFile(const char* path) {
  // Check extension
  const char* ext = strrchr(path, '.');
  if (ext && (strcasecmp(ext, ".svg") == 0)) {
    return LoadSVG(path, 0, 0);  // natural size, caller can ScaleType
  }
  // Existing Skia decode for PNG/JPEG/WebP
  ...
}
```

## API Contract

- `Image::FromFile(path)` returns non-null for valid `.svg` files — caller gets a rasterized Image
- Rasterization size: 0×0 means use SVG natural viewBox size; caller applies ScaleType at render time via ImageWidget
- Memory: nanosvg rasterized pixels are copied into SkBitmap; nanosvg internal data freed after rasterization
- Thread safety: nanosvg is NOT thread-safe per-file — each call creates its own parser instance (fine for Glide worker thread usage)
